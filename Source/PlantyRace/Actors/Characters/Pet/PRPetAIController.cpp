// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PRPetAIController.h"


// Sets default values
APRPetAIController::APRPetAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAttachToPawn = true;
}

// Called when the game starts or when spawned
void APRPetAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APRPetAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

