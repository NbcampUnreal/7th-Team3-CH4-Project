// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PRRotatingActor.generated.h"

class USceneComponent;
class UArrowComponent;
class UStaticMeshComponent;

UCLASS()
class PLANTYRACE_API APRRotatingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APRRotatingActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ServerStartTime();

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* SceneArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	// 회전 속도 조절
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate")
	float DegreesPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate")
	float StartYaw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate")
	FVector SceneOffset = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackPower = 550.f;

	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackUpPower = 220.f;

	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackDownDuration = 1.f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerStartTime)
	float ServerStartTime;

	bool bCanRotate = false;

	void UpdateRotationFromServer();
};
