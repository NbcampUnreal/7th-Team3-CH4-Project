// PRPlayerState.cpp
#include "PRPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Core/PRGameStateBase.h"
#include "GameMode/PRGMB.h"

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
	bFinalWinner = false;
	bEliminated = false;
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

	DOREPLIFETIME(APRPlayerState, SavedClothesData);
	DOREPLIFETIME(APRPlayerState, bHasSavedClothesData);
}

void APRPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	APRPlayerState* NewPS = Cast<APRPlayerState>(PlayerState);
	if (!NewPS)
	{
		return;
	}

	NewPS->SavedClothesData = SavedClothesData;
	NewPS->bHasSavedClothesData = bHasSavedClothesData;
}

void APRPlayerState::ServerRequestOvergrow_Implementation()
{
	GrowthRate = 0.0f;
	UpdateGrowthScoreFromRate();
}

void APRPlayerState::SetReady(bool bNewReady)
{
	if (HasAuthority())
	{
		bIsReady = bNewReady;

		APRGameStateBase* GS = GetWorld()->GetGameState<APRGameStateBase>();
		if (IsValid(GS))
		{
			GS->CheackAllPlayersReady();
		}

		APRGMB* GM = GetWorld()->GetAuthGameMode<APRGMB>();
		if (IsValid(GM))
		{
			if (GS && GS->bAllPlayersReady)
			{
				GM->TryStartLobbyMatch();
			}
			else
			{
				GM->CancelLobbyMatchStart();
			}
		}
	}
	else
	{
		ServerSetReady(bNewReady);
	}
}

void APRPlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;

	APRGameStateBase* GS = GetWorld()->GetGameState<APRGameStateBase>();
	if (IsValid(GS))
	{
		GS->CheackAllPlayersReady();
	}

	APRGMB* GM = GetWorld()->GetAuthGameMode<APRGMB>();
	if (IsValid(GM))
	{
		if (GS && GS->bAllPlayersReady)
		{
			GM->TryStartLobbyMatch();
		}
		else
		{
			GM->CancelLobbyMatchStart();
		}
	}
}