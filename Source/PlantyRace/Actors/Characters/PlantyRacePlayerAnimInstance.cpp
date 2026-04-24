// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PlantyRacePlayerAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "PlantyRaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlantyRacePlayerAnimInstance::AnimNotify_LeftFootStep()
{
	Owner = Cast<APlantyRaceCharacter>(TryGetPawnOwner());
	if (Owner)
	{
		Owner->PlayFootstepSounds(EFootType::Left);
	}
}

void UPlantyRacePlayerAnimInstance::AnimNotify_RightFootStep()
{
	Owner = Cast<APlantyRaceCharacter>(TryGetPawnOwner());
	if (Owner)
	{
		Owner->PlayFootstepSounds(EFootType::Right);
	}
}

void UPlantyRacePlayerAnimInstance::AnimNotify_JumpSound()
{
	Owner = Cast<APlantyRaceCharacter>(TryGetPawnOwner());
	if (Owner)
	{
		Owner->PlayJumpSounds();
	}
}

void UPlantyRacePlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Owner = Cast<APlantyRaceCharacter>(GetOwningActor());

	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
	
	
}

void UPlantyRacePlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!IsValid(Owner))
	{
		return;
	}

	bIsKnockedDown = Owner->IsKnockedDown();
	
	if (Movement)
	{
		Velocity = Movement->Velocity;
		MoveSpeed = Velocity.Size2D();
		isFalling = Movement->IsFalling();
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Owner->GetActorRotation());
		SlopeAngle = Owner->GetFloorSlopeAngle();
		SlopeDirection = Owner->GetSlopeMoveDirectionDot();
		VerticalSpeed = Velocity.Z;	
	}
}