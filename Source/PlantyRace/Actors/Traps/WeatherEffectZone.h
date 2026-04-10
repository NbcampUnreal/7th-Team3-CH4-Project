// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PRGameStateBase.h"
#include "Types/WeatherEffectTypes.h"
#include "WeatherEffectZone.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class PLANTYRACE_API AWeatherEffectZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeatherEffectZone();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone")
	TObjectPtr<USceneComponent> Root;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone")
	TObjectPtr<UBoxComponent> BoxComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone")
	TObjectPtr<UStaticMeshComponent> ZoneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	ESurfaceEffectType SurfaceEffectType = ESurfaceEffectType::Mud;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float MoveSpeedMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float JumpMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	bool bBlockJump = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EWeatherState ZoneWeatherState = EWeatherState::None;

	UPROPERTY(EditAnywhere, Category = "TornadoMove")
	bool bMoveTornado = true;

	UPROPERTY(EditAnywhere, Category = "TornadoMove")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "TornadoMove")
	FVector MoveDirection = FVector::ForwardVector;

	FVector StartLocation;

	UPROPERTY(EditAnywhere, Category = "TornadoMove")
	float MoveRange = 1000.f;
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void HandleWeatherChanged();

	void UpdateZoneVisibilityByWeather(bool bShouldActivate);
};
