// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PRGameStateBase.h"
#include "PRGameModeBase.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class APRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Constructor */
	APRGameModeBase();

	virtual void BeginPlay() override;

protected:
    // 날씨 변경 주기
    UPROPERTY(EditDefaultsOnly, Category = "Weather")
    float WeatherChangeInterval = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Weather")
    float WeatherDuration = 15.f;

    // 날씨 변경 타이머 핸들
    FTimerHandle WeatherChangeTimerHandle;
    // 날씨 초기화
    FTimerHandle WeatherResetTimerHandle;

    bool bWeatherActive = false;

    void StartWeatherTimer();

    void ChangeWeatherPeriodically();

    void ResetWeather();

    EWeatherState GetRandomWeather() const;
};



