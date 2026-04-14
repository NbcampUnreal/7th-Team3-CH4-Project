// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PRCarrotPet.h"


// Sets default values
APRCarrotPet::APRCarrotPet()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APRCarrotPet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APRCarrotPet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APRCarrotPet::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

