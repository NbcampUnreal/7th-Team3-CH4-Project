// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "Core/PRGameStateBase.h"
#include "Actors/Characters/PlantyRacePlayerController.h"
#include "Net/UnrealNetwork.h"

APRGameStateBase::APRGameStateBase()
{
    CurrentWeather = EWeatherState::None;
    RoundNumber = 1;
    RemainingTime = 0.0f;
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
}

void APRGameStateBase::SetWeather(EWeatherState NewWeather)
{
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