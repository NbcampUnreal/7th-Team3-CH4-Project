#include "PRPlayerState.h"
#include "Net/UnrealNetwork.h"

APRPlayerState::APRPlayerState()
{
	TotalScore = 0.0f;
	RaceScore = 0.0f;
	GrowthScore = 0.0f;
	GrowthRate = 0.0f;
	bEliminated = false;

	FinishRank = 0;
	bFinished = false;
	bQualified = false;
	bFinalWinner = false;

	bIsReady = false;
	bIsHost = false;
}

void APRPlayerState::ResetRoundState()
{
	FinishRank = 0;
	bFinished = false;
	bQualified = false;
	bFinalWinner = false;
	bEliminated = false;

	RaceScore = 0.0f;
	GrowthScore = 0.0f;
	TotalScore = 0.0f;
}

void APRPlayerState::UpdateGrowthScoreFromRate()
{
	GrowthScore = GrowthRate;
	TotalScore = RaceScore + GrowthScore;
}

void APRPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APRPlayerState, TotalScore);
	DOREPLIFETIME(APRPlayerState, RaceScore);
	DOREPLIFETIME(APRPlayerState, GrowthScore);
	DOREPLIFETIME(APRPlayerState, GrowthRate);
	DOREPLIFETIME(APRPlayerState, bEliminated);

	DOREPLIFETIME(APRPlayerState, FinishRank);
	DOREPLIFETIME(APRPlayerState, bFinished);
	DOREPLIFETIME(APRPlayerState, bQualified);
	DOREPLIFETIME(APRPlayerState, bFinalWinner);

	DOREPLIFETIME(APRPlayerState, bIsReady);
	DOREPLIFETIME(APRPlayerState, bIsHost);
}

void APRPlayerState::ServerRequestOvergrow_Implementation()
{
	GrowthRate = 0.0f;
	UpdateGrowthScoreFromRate();
}

void APRPlayerState::ServerSetReady_Implementation(bool bReady)
{
	bIsReady = bReady;
}