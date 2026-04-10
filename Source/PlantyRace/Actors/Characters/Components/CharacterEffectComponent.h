// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/WeatherEffectTypes.h"
#include "CharacterEffectComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLANTYRACE_API UCharacterEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterEffectComponent();

	float GetMoveSpeedMultiplier() const;
	float GetJumpMultiplier() const;
	bool GetBlockJump() const;
	ESurfaceEffectType GetCurrentSurfaceEffect() const;

	void SetWeatherState(EWeatherState NewWeatherState);
	void ApplyZoneEffect(
		ESurfaceEffectType InSurfaceEffectType,
		float InMoveSpeedMultiplier,
		float InJumpMultiplier,
		bool bInBlockJump
	);
	void ClearZoneEffect();

	void InitializeEffects();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Weather")
	EWeatherState CurrentWeather = EWeatherState::None;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	ESurfaceEffectType CurrentSurfaceEffect = ESurfaceEffectType::None;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	float ZoneMoveSpeedMultiplier = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	float ZoneJumpMultiplier = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	bool bZoneBlockJump = false;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	ESurfaceEffectType CachedSurfaceEffectType = ESurfaceEffectType::None;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	float CachedMoveSpeedMultiplier = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	float CachedJumpMultiplier = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Weather")
	bool bWeatherBlockJump = false;

	UPROPERTY(VisibleAnywhere, Category = "Weather")
	bool bCachedBlockJump = false;

	UPROPERTY(VisibleAnywhere, Category = "Weather")
	float WeatherMoveSpeedMultiplier = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Weather")
	float WeatherJumpMultiplier = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Weather")
	float DefaultGroundFriction = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Weather")
	float DefaultBrakingDecelerationWalking = 0.f;

	UPROPERTY(EditAnywhere, Category = "Weather")
	float RainGroundFriction = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Weather")
	float RainBrakingDecelerationWalking = 100.f;

private:
	void UpdateWeatherModifiers();
	void RecalculateEffects();
	void ApplyMovementPhysics();
	
};
