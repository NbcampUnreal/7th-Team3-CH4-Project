// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PRPetAIController.generated.h"

UCLASS()
class PLANTYRACE_API APRPetAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APRPetAIController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
