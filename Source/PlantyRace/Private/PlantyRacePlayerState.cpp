// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PlantyRacePlayerState.h"
#include "Net/UnrealNetwork.h"

APlantyRacePlayerState::APlantyRacePlayerState()
{
	PlayerScore = 0.0f;
	GrowthRate = 0.0f;
	bEliminated = false;
}

void APlantyRacePlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlantyRacePlayerState, PlayerScore);
	DOREPLIFETIME(APlantyRacePlayerState, GrowthRate);
	DOREPLIFETIME(APlantyRacePlayerState, bEliminated);
}
