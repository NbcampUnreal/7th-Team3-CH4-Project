#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PRRollingActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class PLANTYRACE_API APRRollingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APRRollingActor();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Stone")
	float MinStartImpulse = 200.f;

	UPROPERTY(EditAnywhere, Category = "Stone")
	float MaxStartImpulse = 500.f;

	UPROPERTY(EditAnywhere, Category = "Stone")
	float KnockbackPower = 800.f;

	UPROPERTY(EditAnywhere, Category = "Stone")
	float KnockdownTime = 1.f;

	UPROPERTY(EditAnywhere, Category = "Stone")
	float LifeSeconds = 5.f;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnActorHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	void ApplyStartImpulse();

	FVector GetInitialRollDirection() const;
};
