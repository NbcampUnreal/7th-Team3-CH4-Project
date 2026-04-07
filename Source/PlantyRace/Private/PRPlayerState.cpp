// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PRPlayerState.h"
#include "Net/UnrealNetwork.h"

APRPlayerState::APRPlayerState()
{
	PlayerScore = 0.0f;
	GrowthRate = 0.0f;
	bEliminated = false;
}

void APRPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APRPlayerState, PlayerScore);
	DOREPLIFETIME(APRPlayerState, GrowthRate);
	DOREPLIFETIME(APRPlayerState, bEliminated);
}
