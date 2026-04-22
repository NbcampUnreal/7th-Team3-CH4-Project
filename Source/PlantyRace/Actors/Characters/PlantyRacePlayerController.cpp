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
#include "Core/PRPlayerState.h"


void APlantyRacePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(!IsLocalController()) return;

	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;

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