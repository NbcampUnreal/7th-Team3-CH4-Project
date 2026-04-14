// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Characters/PRPetCharacter.h"
#include "PRCarrotPet.generated.h"

UCLASS()
class PLANTYRACE_API APRCarrotPet : public APRPetCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APRCarrotPet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
