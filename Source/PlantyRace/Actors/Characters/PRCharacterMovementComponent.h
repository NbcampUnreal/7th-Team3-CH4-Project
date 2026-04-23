// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PRCharacterMovementComponent.generated.h"

class APlantyRaceCharacter;

UCLASS()
class PLANTYRACE_API UPRCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
protected:
	virtual void InitializeComponent() override;
	virtual void OnMovementUpdated(
		float DeltaSeconds,
		const FVector& OldLocation,
		const FVector& OldVelocity
	) override;

private:
	TObjectPtr<APlantyRaceCharacter> CachedCharacter = nullptr;
};
