// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Types/WeatherEffectTypes.h"
#include "PRGameStateBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnWeatherChanged);

UCLASS()
class PLANTYRACE_API APRGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	APRGameStateBase();

	UFUNCTION()
	void OnRep_WeatherState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetWeather(EWeatherState NewWeather);

	EWeatherState GetCurrentWeather() const { return CurrentWeather; }

	UFUNCTION(BlueprintCallable)
	FText GetWeatherText() const;

	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(ReplicatedUsing = OnRep_RoundNumber,BlueprintReadOnly, Category = "Round")
	int32 RoundNumber;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly, Category = "Round")
	float RemainingTime;

	UFUNCTION()
	void OnRep_RoundNumber();

	UFUNCTION()
	void OnRep_RemainingTime();

	void SetRoundNumber(int32 NewRound);
	void SetRemainingTime(float NewTime);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_WeatherState, BlueprintReadOnly)
	EWeatherState CurrentWeather;

};
