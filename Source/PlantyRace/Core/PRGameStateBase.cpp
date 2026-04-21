// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PRGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/Characters/PlantyRacePlayerController.h"
#include "PRPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "BluePrint/UserWidget.h"
#include "Audio/PRSoundManager.h"

APRGameStateBase::APRGameStateBase()
{
    CurrentWeather = EWeatherState::None;
    RoundNumber = 1;
    RemainingTime = 0.0f;
    SoundManager = nullptr;
    bAllPlayersReady = false;
}

void APRGameStateBase::OnRep_WeatherState()
{
    OnWeatherChanged.Broadcast();
}

void APRGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, CurrentWeather);
    DOREPLIFETIME(ThisClass, RoundNumber);
    DOREPLIFETIME(ThisClass, RemainingTime);
    DOREPLIFETIME(ThisClass, SoundManager);
    DOREPLIFETIME(ThisClass, bAllPlayersReady);
}

void APRGameStateBase::SetWeather(EWeatherState NewWeather)
{
    if (!HasAuthority())
    {
        return;
    }

    if (CurrentWeather == NewWeather)
    {
        return;
    }

    CurrentWeather = NewWeather;

    OnWeatherChanged.Broadcast();
}

FText APRGameStateBase::GetWeatherText() const
{
    switch (CurrentWeather)
    {
    case EWeatherState::None:
        return FText::FromString(TEXT("Normal"));
    case EWeatherState::Rain:
        return FText::FromString(TEXT("Rain"));
    case EWeatherState::Sun:
        return FText::FromString(TEXT("Sun"));
    case EWeatherState::Tornado:
        return FText::FromString(TEXT("Tornado"));
    default:
        return FText::FromString(TEXT("Unknown"));
    }
}

void APRGameStateBase::OnRep_RoundNumber()
{
    //UI 업데이트
}

void APRGameStateBase::OnRep_RemainingTime()
{
    //UI 업데이트
}

void APRGameStateBase::SetRoundNumber(int32 NewRound)
{
    RoundNumber = NewRound;
    OnRep_RoundNumber();
}

void APRGameStateBase::SetRemainingTime(float NewTime)
{
    RemainingTime = NewTime;
    OnRep_RemainingTime();
}

void APRGameStateBase::SetSoundManager(APRSoundManager* InSoundManager)
{
    if (!HasAuthority())
    {
        return;
    }

    SoundManager = InSoundManager;
}

APRSoundManager* APRGameStateBase::GetSoundManager() const
{
    return SoundManager;
}

void APRGameStateBase::UpdateHUD()
{
    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        APlantyRacePlayerController* PlantyRacePlayerController = Cast<APlantyRacePlayerController>(PlayerController);

        if (PlantyRacePlayerController)
        {
            APRPlayerState* APRPlayerStateInstance = Cast<APRPlayerState>(PlantyRacePlayerController->PlayerState);

            if (UUserWidget* HUDWidget = PlantyRacePlayerController->GetHUDWidget())
            {
                /*if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
                {
                    float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
                    TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
                }*/

                if (APRPlayerStateInstance)
                {
                    if (UProgressBar* GrowthProgressBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("GrowthProgressBar"))))
                    {
                        float MaxGrowth = APRPlayerStateInstance->MaxGrowthRate;
                        float CurrentGrowth = APRPlayerStateInstance->GrowthRate;

                        float Percent = (MaxGrowth > 0.0f) ? (CurrentGrowth / MaxGrowth) : 0.0f;
                        GrowthProgressBar->SetPercent(Percent);
                    }
                }

                if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
                {
                    LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("round: %d '/2'"), RoundNumber)));
                }
            }
        }
    }
}

void APRGameStateBase::OnRep_AllPlayersReady()
{
    //게임 시작 버튼 연결
}

void APRGameStateBase::CheackAllPlayersReady()
{
    if (!HasAuthority())return;
    for (APlayerState* PS : PlayerArray)
    {
        APRPlayerState* PRS = Cast<APRPlayerState>(PS);
        if(PRS && !PRS->bIsReady)
        {
            bAllPlayersReady = false;
            return;
        }
    }
    bAllPlayersReady = true;
    OnRep_AllPlayersReady();
}