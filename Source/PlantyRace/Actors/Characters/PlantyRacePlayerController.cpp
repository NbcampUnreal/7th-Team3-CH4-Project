// Copyright Epic Games, Inc. All Rights Reserved.


#include "PlantyRacePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "PlantyRace.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UI/PRWeatherWidget.h"
#include "Core/PRGameStateBase.h"
#include "UI/UW_GameResult.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Core/PRPlayerState.h"


void APlantyRacePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(!IsLocalController()) return;
	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

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
}

void APlantyRacePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
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

				FString GameResultString = FString::Printf(TEXT("%02d : "), InRanking); //, Printf(TEXT("%s"), APRPlayerState->TotalScore);
				GameResultUI->ResultText->SetText(FText::FromString(GameResultString));

				FString RankingString = FString::Printf(TEXT("#%02d"), InRanking);
				GameResultUI->RankingText->SetText(FText::FromString(RankingString));

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