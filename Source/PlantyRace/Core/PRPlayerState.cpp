#include "PRPlayerState.h"
#include "Net/UnrealNetwork.h"

APRPlayerState::APRPlayerState()
{
	PlayerScore = 0.0f;
	GrowthRate = 0.0f;
	bEliminated = false;

	FinishRank = 0;
	bFinished = false;
	bQualified = false;
	bFinalWinner = false;
}

void APRPlayerState::ResetRoundState()
{
	FinishRank = 0;
	bFinished = false;
}

void APRPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APRPlayerState, PlayerScore);
	DOREPLIFETIME(APRPlayerState, GrowthRate);
	DOREPLIFETIME(APRPlayerState, bEliminated);

	DOREPLIFETIME(APRPlayerState, FinishRank);
	DOREPLIFETIME(APRPlayerState, bFinished);
	DOREPLIFETIME(APRPlayerState, bQualified);
	DOREPLIFETIME(APRPlayerState, bFinalWinner);
}

void APRPlayerState::ServerRequestOvergrow_Implementation()
{
	GrowthRate = 0.0f;
}