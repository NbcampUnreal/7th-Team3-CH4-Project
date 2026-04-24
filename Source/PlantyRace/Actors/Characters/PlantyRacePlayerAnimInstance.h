// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlantyRacePlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PLANTYRACE_API UPlantyRacePlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION()
	void AnimNotify_LeftFootStep();

	UFUNCTION()
	void AnimNotify_RightFootStep();
	
	UFUNCTION()
	void AnimNotify_JumpSound();

public:
	virtual void NativeInitializeAnimation() override; // 애니메이션이 생성되면 호출되는 함수.

	virtual void NativeUpdateAnimation(float DeltaSeconds) override; // 프레임마다 호출되는 함수.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class APlantyRaceCharacter* Owner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class UCharacterMovementComponent* Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	FVector Velocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	float VerticalSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	bool isFalling = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	float Direction;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float SlopeAngle;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float SlopeDirection;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsKnockedDown = false;
};

