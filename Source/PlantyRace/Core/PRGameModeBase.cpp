//// Copyright Epic Games, Inc. All Rights Reserved.
//
//#include "PRGameModeBase.h"
//
//APRGMB::APRGMB()
//{
//	
//}
//
//void APRGMB::BeginPlay()
//{
//    Super::BeginPlay();
//
//    APRGameStateBase* GS = GetGameState<APRGameStateBase>();
//    if (IsValid(GS) == true)
//    {
//        GS->SetWeather(EWeatherState::None);
//
//        StartWeatherTimer();
//    }
//}
//
//void APRGMB::StartWeatherTimer()
//{
//    GetWorldTimerManager().SetTimer(
//        WeatherChangeTimerHandle,
//        this,
//        &APRGMB::ChangeWeatherPeriodically,
//        WeatherChangeInterval,
//        true
//    );
//}
//
//void APRGMB::ChangeWeatherPeriodically()
//{
//    if (bWeatherActive == true)
//    {
//        return;
//    }
//
//    APRGameStateBase* GS = GetGameState<APRGameStateBase>();
//    if (IsValid(GS) == true)
//    {
//        EWeatherState NewWeather = GetRandomWeather();
//
//        GS->SetWeather(NewWeather);
//
//        if (NewWeather != EWeatherState::None)
//        {
//            bWeatherActive = true;
//
//            GetWorldTimerManager().SetTimer(
//                WeatherResetTimerHandle,
//                this,
//                &APRGMB::ResetWeather,
//                WeatherDuration,
//                false
//            );
//        }
//    }
//}
//
//void APRGMB::ResetWeather()
//{
//    APRGameStateBase* GS = GetGameState<APRGameStateBase>();
//    if (IsValid(GS) == true)
//    {
//        GS->SetWeather(EWeatherState::None);
//        
//        bWeatherActive = false;
//    }
//}
//
//EWeatherState APRGMB::GetRandomWeather() const
//{
//    int32 ChangeRoll = FMath::RandRange(0, 1);
//
//    if (ChangeRoll != 0)
//    {
//        return EWeatherState::None;
//    }
//
//    int32 WeatherRoll = FMath::RandRange(0, 2);
//
//    if (WeatherRoll == 0)
//    {
//        return EWeatherState::Rain;
//    }
//    else if (WeatherRoll == 1)
//    {
//        return EWeatherState::Sun;
//    }
//
//    return EWeatherState::Tornado;
//}
