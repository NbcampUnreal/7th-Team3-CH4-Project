// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Types/WeatherEffectTypes.h"
#include "PRGameStateBase.generated.h"

class APRSoundManager;

DECLARE_MULTICAST_DELEGATE(FOnWeatherChanged);

UCLASS()
class PLANTYRACE_API APRGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	APRGameStateBase();

	UFUNCTION()

	EWeatherState GetCurrentWeather() const { return CurrentWeather; }

	UFUNCTION(BlueprintCallable)
	FText GetWeatherText() const;

	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(ReplicatedUsing = OnRep_RoundNumber, BlueprintReadOnly, Category = "Round")
	int32 RoundNumber;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly, Category = "Round")
	float RemainingTime;

	UPROPERTY(ReplicatedUsing = OnRep_AllPlayersReady, BlueprintReadOnly, Category = "Lobby")
	bool bAllPlayersReady;

	UFUNCTION()
	void OnRep_AllPlayersReady();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void CheackAllPlayersReady();

	UFUNCTION()
	void OnRep_RoundNumber();

	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetSoundManager(APRSoundManager* InSoundManager);

	UFUNCTION(BlueprintPure, Category = "Audio")
	APRSoundManager* GetSoundManager() const;

	void OnRep_WeatherState();
	void SetWeather(EWeatherState NewWeather);
	void SetRoundNumber(int32 NewRound);
	void SetRemainingTime(float NewTime);
	void UpdateHUD();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_WeatherState, BlueprintReadOnly)
	EWeatherState CurrentWeather;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<APRSoundManager> SoundManager;
};