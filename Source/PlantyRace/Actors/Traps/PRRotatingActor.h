// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PRRotatingActor.generated.h"

class USceneComponent;
class UArrowComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class AActor;

UENUM(BlueprintType)
enum class ERotationAxis : uint8
{
	X	UMETA(DisplayName = "X"),
	Y	UMETA(DisplayName = "Y"),
	Z	UMETA(DisplayName = "Z")
};

UCLASS()
class PLANTYRACE_API APRRotatingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APRRotatingActor();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate")
	ERotationAxis RotationAxis = ERotationAxis::Z;

	// 회전 속도 조절
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate")
	float DegreesPerSecond = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate")
	float StartAngle = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate")
	FVector MeshOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackPower = 550.f;

	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackUpPower = 220.f;

	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackDownDuration = 1.f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerStartTime)
	float ServerStartTime = -1.f;

	bool bCanRotate = false;

protected:
	float GetCurrentAxisAngle() const;
	FRotator RotationFromAngle(float Angle) const;
	FVector PlanarDirection(const FVector& Source) const;
	FVector TangentDirection(const FVector& RadiusDir) const;
	void UpdateRotationFromServer();
};
