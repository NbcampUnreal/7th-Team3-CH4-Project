// Copyright Epic Games, Inc. All Rights Reserved.

#include "PRGameModeBase.h"

APRGameModeBase::APRGameModeBase()
{
	// stub
}

void APRGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    APRGameStateBase* GS = GetGameState<APRGameStateBase>();
    if (IsValid(GS) == true)
    {
        GS->SetWeather(EWeatherState::None);

        StartWeatherTimer();
    }
}

void APRGameModeBase::StartWeatherTimer()
{
    GetWorldTimerManager().SetTimer(
        WeatherChangeTimerHandle,
        this,
        &APRGameModeBase::ChangeWeatherPeriodically,
        WeatherChangeInterval,
        true
    );
}

void APRGameModeBase::ChangeWeatherPeriodically()
{
    if (bWeatherActive == true)
    {
        return;
    }

    APRGameStateBase* GS = GetGameState<APRGameStateBase>();
    if (IsValid(GS) == true)
    {
        EWeatherState NewWeather = GetRandomWeather();

        GS->SetWeather(NewWeather);

        if (NewWeather != EWeatherState::None)
        {
            bWeatherActive = true;

            GetWorldTimerManager().SetTimer(
                WeatherResetTimerHandle,
                this,
                &APRGameModeBase::ResetWeather,
                WeatherDuration,
                false
            );
        }
    }
}

void APRGameModeBase::ResetWeather()
{
    APRGameStateBase* GS = GetGameState<APRGameStateBase>();
    if (IsValid(GS) == true)
    {
        GS->SetWeather(EWeatherState::None);
        
        bWeatherActive = false;
    }
}

EWeatherState APRGameModeBase::GetRandomWeather() const
{
    int32 ChangeRoll = FMath::RandRange(0, 1);

    if (ChangeRoll != 0)
    {
        return EWeatherState::None;
    }

    int32 WeatherRoll = FMath::RandRange(0, 2);

    if (WeatherRoll == 0)
    {
        return EWeatherState::Rain;
    }
    else if (WeatherRoll == 1)
    {
        return EWeatherState::Sun;
    }

    return EWeatherState::Tornado;
}
