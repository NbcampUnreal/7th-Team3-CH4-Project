// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PRGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/Characters/PlantyRacePlayerController.h"
#include "PRPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BluePrint/UserWidget.h"
#include "Audio/PRSoundManager.h"

APRGameStateBase::APRGameStateBase()
{
    CurrentWeather = EWeatherState::None;
    RoundNumber = 1;
    RemainingTime = 180.0f;
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
    APlantyRacePlayerController* PC = Cast<APlantyRacePlayerController>(GetWorld()->GetFirstPlayerController());

    if (PC)
    {
        PC->UpdateHUD();
    }
}

void APRGameStateBase::OnRep_RemainingTime()
{
    APlantyRacePlayerController* PC = Cast<APlantyRacePlayerController>(GetWorld()->GetFirstPlayerController());

    if (PC)
    {
        PC->UpdateHUD();
    }
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


void APRGameStateBase::OnRep_AllPlayersReady()
{
    //게임 시작 버튼 연결
}

void APRGameStateBase::CheackAllPlayersReady()
{
    if (!HasAuthority())
    {
        return;
    }

    bAllPlayersReady = false;

    if (PlayerArray.Num() <= 0)
    {
        OnRep_AllPlayersReady();
        return;
    }

    for (APlayerState* PS : PlayerArray)
    {
        APRPlayerState* PRS = Cast<APRPlayerState>(PS);
        if (!PRS || !PRS->bIsReady)
        {
            bAllPlayersReady = false;
            OnRep_AllPlayersReady();
            return;
        }
    }

    bAllPlayersReady = true;
    OnRep_AllPlayersReady();
}