#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PRPigCharacter.generated.h"

class UBoxComponent;
class APlantyRaceCharacter;
class UPRKnockbackComponent;

UCLASS()
class PLANTYRACE_API APRPigCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APRPigCharacter();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Pig|Collision")
	UBoxComponent* HitBox;

	UPROPERTY(EditAnywhere, Category = "Pig")
	float KnockbackPower = 800.f;

	UPROPERTY(EditAnywhere, Category = "Pig")
	float KnockbackUpPower = 250.f;

	UPROPERTY(EditAnywhere, Category = "Pig")
	float DownDuration = 1.f;

	UPROPERTY(EditAnywhere, Category = "Pig|Move")
	float MoveRadius = 800.f;

	UPROPERTY(EditAnywhere, Category = "Pig|Move")
	float MoveInterval = 2.f;

	UPROPERTY(VisibleAnywhere, Category = "Pig|Move")
	FVector InitialLocation;

	FTimerHandle RandomMoveTimerHandle;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPigHitOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	FVector CalculateKnockbackVelocity(const APlantyRaceCharacter* TargetCharacter);
	
	void ApplyKnockback(APlantyRaceCharacter* TargetCharacter);

	void StartRandomMove();

	void MoveToRandomLocation();

};
