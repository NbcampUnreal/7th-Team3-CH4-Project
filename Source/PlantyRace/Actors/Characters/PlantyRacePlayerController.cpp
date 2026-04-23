#include "PlantyRacePlayerController.h"
#include "Audio/PRSoundManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/PRGameStateBase.h"
#include "Core/PRPlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameMode/PRGMB.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "PlantyRace.h"
#include "UI/PRWeatherWidget.h"
#include "UI/UW_GameResult.h"
#include "Widgets/Input/SVirtualJoystick.h"


void APlantyRacePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;

	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogPlantyRace, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}

	UpdateHUD();
	CreateWeatherWidget();

	APRGameStateBase* PGS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(PGS))
	{
		return;
	}

	PGS->OnWeatherChanged.AddUObject(this, &APlantyRacePlayerController::HandleWeatherChanged);
	HandleWeatherChanged();

	const FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);

	if (MapName == TEXT("L_Title"))
	{
		ClientPlayMapBGM(EPRBGMType::Title);
	}
	else if (MapName == TEXT("L_Lobby"))
	{
		ClientPlayMapBGM(EPRBGMType::Lobby);
	}
	else if (MapName == TEXT("L_Round1"))
	{
		ClientPlayMapBGM(EPRBGMType::Round1);
	}
	else if (MapName == TEXT("L_Round2"))
	{
		ClientPlayMapBGM(EPRBGMType::Round2);
	}
	else if (MapName == TEXT("L_Result"))
	{
		ClientPlayMapBGM(EPRBGMType::Result);
	}
}
void APlantyRacePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (SpectatePrevAction)
		{
			EnhancedInput->BindAction(SpectatePrevAction, ETriggerEvent::Started, this, &APlantyRacePlayerController::SpectatePrevPlayer);
		}

		if (SpectateNextAction)
		{
			EnhancedInput->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &APlantyRacePlayerController::SpectateNextPlayer);
		}
	}
}

void APlantyRacePlayerController::CreateWeatherWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!WeatherWidgetClass)
	{
		return;
	}

	UPRWeatherWidget* NewWeatherWidget = CreateWidget<UPRWeatherWidget>(this, WeatherWidgetClass);
	WeatherWidget = NewWeatherWidget;

	if (!IsValid(WeatherWidget))
	{
		return;
	}

	WeatherWidget->AddToViewport();
	UpdateWeatherUI();
}

void APlantyRacePlayerController::UpdateWeatherUI()
{
	if (!IsValid(WeatherWidget))
	{
		return;
	}

	AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	APRGameStateBase* PGS = Cast<APRGameStateBase>(GS);
	if (!IsValid(PGS))
	{
		return;
	}

	WeatherWidget->SetWeatherText(PGS->GetWeatherText());
}

void APlantyRacePlayerController::HandleWeatherChanged()
{
	UpdateWeatherUI();
}

UUserWidget* APlantyRacePlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

void APlantyRacePlayerController::ClientRPCShowGameResultWidget_Implementation(int32 InRanking)
{
	if (IsLocalController() == true)
	{
		if (IsValid(GameResultUIClass) == true)
		{
			UUW_GameResult* GameResultUI = CreateWidget<UUW_GameResult>(this, GameResultUIClass);
			if (IsValid(GameResultUI) == true)
			{
				GameResultUI->AddToViewport(3);

				//FString GameResultString = FString::Printf(TEXT("%02d : "), InRanking); //, Printf(TEXT("%s"), APRPlayerState->TotalScore);
				//GameResultUI->ResultText->SetText(FText::FromString(GameResultString));

				//FString RankingString = FString::Printf(TEXT("#%02d"), InRanking);
				//GameResultUI->RankingText->SetText(FText::FromString(RankingString));

				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(GameResultUI->GetCachedWidget());
				SetInputMode(Mode);

				bShowMouseCursor = true;
			}
		}
	}
}


void APlantyRacePlayerController::UpdateHUD()
{
	if (!IsLocalController()) return;

	UUserWidget* HUDWidget = GetHUDWidget(); 
	if (!IsValid(HUDWidget)) return;

	if (APRGameStateBase* GS = GetWorld()->GetGameState<APRGameStateBase>())
	{
		if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
		{
			int32 Minutes = FMath::FloorToInt(GS->RemainingTime / 60.0f);
			int32 Seconds = FMath::FloorToInt(GS->RemainingTime) % 60;

			TimeText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
		}

		if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("round"))))
		{
			LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("round: %d / 2"), GS->RoundNumber)));
		}
	}

	APRPlayerState* PS = GetPlayerState<APRPlayerState>();
	if (IsValid(PS))
	{
		if (UProgressBar* GrowthProgressBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("GrowthProgressBar"))))
		{
			float MaxGrowth = PS->MaxGrowthRate;
			float CurrentGrowth = PS->GrowthRate;
			float Percent = (MaxGrowth > 0.0f) ? (CurrentGrowth / MaxGrowth) : 0.0f;
			GrowthProgressBar->SetPercent(Percent);
		}
	}
}

void APlantyRacePlayerController::ClientPlayCheckPointSFX_Implementation(FVector Location)
{
	if (!IsLocalController())
	{
		return;
	}

	APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	APRSoundManager* SM = GS->GetSoundManager();
	if (!IsValid(SM))
	{
		return;
	}

	SM->PlayCheckPointSFX(Location);
}

void APlantyRacePlayerController::ClientPlayRespawnSFX_Implementation(FVector Location)
{
	if (!IsLocalController())
	{
		return;
	}

	APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	APRSoundManager* SM = GS->GetSoundManager();
	if (!IsValid(SM))
	{
		return;
	}

	SM->PlayRespawnSFX(Location);
}

void APlantyRacePlayerController::ClientPlayFinishSFX_Implementation(FVector Location)
{
	if (!IsLocalController())
	{
		return;
	}

	APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	APRSoundManager* SM = GS->GetSoundManager();
	if (!IsValid(SM))
	{
		return;
	}

	USoundBase* FinishSound = SM->GetFinishSFX();
	if (!IsValid(FinishSound))
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, FinishSound);
}

void APlantyRacePlayerController::ClientPlayMapBGM_Implementation(EPRBGMType BGMType)
{
	if (!IsLocalController())
	{
		return;
	}

	APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	APRSoundManager* SM = GS->GetSoundManager();
	if (!IsValid(SM))
	{
		return;
	}

	SM->PlayBGMByType(BGMType);
}
void APlantyRacePlayerController::ClientPlayRoundStartSFX_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	APRSoundManager* SM = GS->GetSoundManager();
	if (!IsValid(SM))
	{
		return;
	}

	SM->PlayRoundStartSFX();
}

bool APlantyRacePlayerController::IsValidSpectateTarget(APlantyRaceCharacter* TargetCharacter) const
{
	if (!IsValid(TargetCharacter))
	{
		return false;
	}

	if (TargetCharacter->IsHidden() || !TargetCharacter->GetActorEnableCollision())
	{
		return false;
	}

	APRPlayerState* TargetPS = TargetCharacter->GetPlayerState<APRPlayerState>();
	if (!IsValid(TargetPS))
	{
		return false;
	}

	if (TargetPS->IsFinished())
	{
		return false;
	}

	if (TargetPS->IsEliminated())
	{
		return false;
	}

	return true;
}

void APlantyRacePlayerController::RefreshSpectateTargets()
{
	SpectateTargets.Empty();

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (TActorIterator<APlantyRaceCharacter> It(World); It; ++It)
	{
		APlantyRaceCharacter* TargetCharacter = *It;
		if (!IsValidSpectateTarget(TargetCharacter))
		{
			continue;
		}

		if (TargetCharacter == GetPawn())
		{
			continue;
		}

		SpectateTargets.Add(TargetCharacter);
	}

	if (SpectateTargets.Num() <= 0)
	{
		CurrentSpectateIndex = INDEX_NONE;

		APawn* MyPawn = GetPawn();
		if (IsValid(MyPawn))
		{
			SetViewTargetWithBlend(MyPawn, 0.2f);
		}
		return;
	}

	if (!SpectateTargets.IsValidIndex(CurrentSpectateIndex))
	{
		CurrentSpectateIndex = 0;
	}
}

void APlantyRacePlayerController::ApplySpectateTargetByIndex(int32 TargetIndex)
{
	if (!SpectateTargets.IsValidIndex(TargetIndex))
	{
		return;
	}

	APlantyRaceCharacter* TargetCharacter = SpectateTargets[TargetIndex];
	if (!IsValidSpectateTarget(TargetCharacter))
	{
		RefreshSpectateTargets();

		if (!SpectateTargets.IsValidIndex(TargetIndex))
		{
			return;
		}

		TargetCharacter = SpectateTargets[TargetIndex];
		if (!IsValidSpectateTarget(TargetCharacter))
		{
			return;
		}
	}

	CurrentSpectateIndex = TargetIndex;
	SetViewTargetWithBlend(TargetCharacter, 0.35f);
}

void APlantyRacePlayerController::StartSpectatingOtherPlayers()
{
	if (!CanSpectate())
	{
		return;
	}

	RefreshSpectateTargets();

	if (SpectateTargets.Num() <= 0)
	{
		return;
	}

	CurrentSpectateIndex = 0;
	ApplySpectateTargetByIndex(CurrentSpectateIndex);
}

void APlantyRacePlayerController::SpectatePrevPlayer()
{
	if (!IsLocalController() || !CanSpectate())
	{
		return;
	}

	RefreshSpectateTargets();

	if (SpectateTargets.Num() <= 0)
	{
		return;
	}

	if (CurrentSpectateIndex == INDEX_NONE)
	{
		CurrentSpectateIndex = 0;
		ApplySpectateTargetByIndex(CurrentSpectateIndex);
		return;
	}

	const int32 PrevIndex =
		(CurrentSpectateIndex - 1 + SpectateTargets.Num()) % SpectateTargets.Num();

	ApplySpectateTargetByIndex(PrevIndex);
}

void APlantyRacePlayerController::SpectateNextPlayer()
{
	if (!IsLocalController() || !CanSpectate())
	{
		return;
	}

	RefreshSpectateTargets();

	if (SpectateTargets.Num() <= 0)
	{
		return;
	}

	if (CurrentSpectateIndex == INDEX_NONE)
	{
		CurrentSpectateIndex = 0;
		ApplySpectateTargetByIndex(CurrentSpectateIndex);
		return;
	}

	const int32 NextIndex =
		(CurrentSpectateIndex + 1) % SpectateTargets.Num();

	ApplySpectateTargetByIndex(NextIndex);
}

bool APlantyRacePlayerController::CanSpectate() const
{
	APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return false;
	}

	// 로비 / 타이틀에서는 관전 금지
	if (GS->RoundNumber <= 0)
	{
		return false;
	}

	APRPlayerState* PS = GetPlayerState<APRPlayerState>();
	if (!IsValid(PS))
	{
		return false;
	}

	// 아직 플레이 중이면 관전 금지
	if (!PS->IsFinished() && !PS->IsEliminated())
	{
		return false;
	}

	return true;
}

void APlantyRacePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!IsLocalController())
	{
		return;
	}

	if (!CanSpectate())
	{
		return;
	}

	if (!SpectateTargets.IsValidIndex(CurrentSpectateIndex))
	{
		return;
	}

	APlantyRaceCharacter* TargetCharacter = SpectateTargets[CurrentSpectateIndex];
	if (!IsValidSpectateTarget(TargetCharacter))
	{
		return;
	}

	if (GetViewTarget() != TargetCharacter)
	{
		SetViewTarget(TargetCharacter);
	}
}