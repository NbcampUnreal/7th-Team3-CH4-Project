#include "GameMode/PRGMB.h"
#include "GameMode/SpawnPoint.h"
#include "Core/PRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"

APRGMB::APRGMB()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APRGMB::BeginPlay()
{
	Super::BeginPlay();

	CollectSpawnPoints();
	StartRound1();
}

void APRGMB::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer && NewPlayer->PlayerState)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(NewPlayer->PlayerState))
		{
			PRPlayerState->ResetRoundState();
		}
	}
}

void APRGMB::CollectSpawnPoints()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPoint::StaticClass(), FoundActors);

	SpawnPoints.Empty();

	for (AActor* FoundActor : FoundActors)
	{
		if (ASpawnPoint* SpawnPoint = Cast<ASpawnPoint>(FoundActor))
		{
			SpawnPoints.Add(SpawnPoint);
		}
	}

	SortSpawnPoints();

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Collected SpawnPoints: %d"), SpawnPoints.Num());
}

void APRGMB::SortSpawnPoints()
{
	SpawnPoints.Sort([](const ASpawnPoint& A, const ASpawnPoint& B)
		{
			return A.SpawnIndex < B.SpawnIndex;
		});
}

ASpawnPoint* APRGMB::GetSpawnPointByIndex(int32 Index) const
{
	if (SpawnPoints.IsValidIndex(Index))
	{
		return SpawnPoints[Index];
	}

	return nullptr;
}

void APRGMB::StartRound1()
{
	ResetRoundData();
	QualifiedPlayers.Empty();

	CurrentRound = EPRMatchRound::Round1;

	UE_LOG(LogTemp, Warning, TEXT("========== ROUND 1 START =========="));
	UE_LOG(LogTemp, Warning, TEXT("Rule: Top %d players qualify."), Round1QualifiedCount);

	if (!GameState)
	{
		return;
	}

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			PRPlayerState->ResetRoundState();
			PRPlayerState->SetEliminated(false);
			PRPlayerState->SetQualified(false);
			PRPlayerState->SetFinalWinner(false);
		}
	}
}

void APRGMB::StartRound2()
{
	ResetRoundData();
	CurrentRound = EPRMatchRound::Round2;

	UE_LOG(LogTemp, Warning, TEXT("========== ROUND 2 START =========="));
	UE_LOG(LogTemp, Warning, TEXT("Qualified players: %d"), QualifiedPlayers.Num());

	if (!GameState)
	{
		return;
	}

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			PRPlayerState->ResetRoundState();

			const bool bQualified = QualifiedPlayers.Contains(PRPlayerState);
			PRPlayerState->SetQualified(bQualified);
			PRPlayerState->SetEliminated(!bQualified);
		}
	}
}

void APRGMB::RegisterPlayerFinish(APRPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}

	if (CurrentRound == EPRMatchRound::Finished || CurrentRound == EPRMatchRound::None)
	{
		return;
	}

	if (PlayerState->IsFinished())
	{
		return;
	}

	if (CurrentRound == EPRMatchRound::Round2 && !QualifiedPlayers.Contains(PlayerState))
	{
		return;
	}

	PlayerState->SetFinished(true);

	const int32 NewRank = FinishOrder.Num() + 1;
	PlayerState->SetFinishRank(NewRank);

	FinishOrder.Add(PlayerState);

	UE_LOG(LogTemp, Warning, TEXT("[Finish] %s finished with rank %d"),
		*PlayerState->GetPlayerName(),
		NewRank);

	PrintFinishOrderLog();

	if (CurrentRound == EPRMatchRound::Round1)
	{
		if (FinishOrder.Num() >= Round1QualifiedCount)
		{
			EndCurrentRound();
		}
	}
	else if (CurrentRound == EPRMatchRound::Round2)
	{
		if (FinishOrder.Num() >= FinalRoundWinnerCount)
		{
			EndCurrentRound();
		}
	}
}

void APRGMB::EndCurrentRound()
{
	if (CurrentRound == EPRMatchRound::Round1)
	{
		UE_LOG(LogTemp, Warning, TEXT("========== ROUND 1 END =========="));
		ProcessRound1Results();
		StartRound2();
	}
	else if (CurrentRound == EPRMatchRound::Round2)
	{
		UE_LOG(LogTemp, Warning, TEXT("========== ROUND 2 END =========="));
		ProcessRound2Results();
		CurrentRound = EPRMatchRound::Finished;
		UE_LOG(LogTemp, Warning, TEXT("========== MATCH FINISHED =========="));
	}
}

void APRGMB::ResetRoundData()
{
	FinishOrder.Empty();

	if (!GameState)
	{
		return;
	}

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			PRPlayerState->ResetRoundState();
		}
	}
}

bool APRGMB::IsPlayerQualified(APRPlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return false;
	}

	return QualifiedPlayers.Contains(PlayerState);
}

void APRGMB::ProcessRound1Results()
{
	QualifiedPlayers.Empty();

	for (int32 i = 0; i < FinishOrder.Num(); ++i)
	{
		if (APRPlayerState* PRPlayerState = FinishOrder[i])
		{
			const bool bQualified = i < Round1QualifiedCount;

			PRPlayerState->SetQualified(bQualified);
			PRPlayerState->SetEliminated(!bQualified);

			if (bQualified)
			{
				QualifiedPlayers.Add(PRPlayerState);
			}
		}
	}

	if (GameState)
	{
		for (APlayerState* BasePlayerState : GameState->PlayerArray)
		{
			if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
			{
				if (!FinishOrder.Contains(PRPlayerState))
				{
					PRPlayerState->SetQualified(false);
					PRPlayerState->SetEliminated(true);
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("----- ROUND 1 RESULT -----"));
	for (APRPlayerState* QualifiedPlayer : QualifiedPlayers)
	{
		if (QualifiedPlayer)
		{
			UE_LOG(LogTemp, Warning, TEXT("Qualified: %s (Rank %d)"),
				*QualifiedPlayer->GetPlayerName(),
				QualifiedPlayer->GetFinishRank());
		}
	}
}

void APRGMB::ProcessRound2Results()
{
	if (FinishOrder.Num() <= 0)
	{
		return;
	}

	for (APRPlayerState* QualifiedPlayer : QualifiedPlayers)
	{
		if (QualifiedPlayer)
		{
			QualifiedPlayer->SetFinalWinner(false);
		}
	}

	if (APRPlayerState* Winner = FinishOrder[0])
	{
		Winner->SetFinalWinner(true);

		UE_LOG(LogTemp, Warning, TEXT("===== FINAL WINNER: %s ====="),
			*Winner->GetPlayerName());
	}
}

void APRGMB::PrintFinishOrderLog() const
{
	UE_LOG(LogTemp, Warning, TEXT("----- Current Finish Order -----"));

	for (int32 i = 0; i < FinishOrder.Num(); ++i)
	{
		const APRPlayerState* PRPlayerState = FinishOrder[i];
		if (PRPlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d. %s"), i + 1, *PRPlayerState->GetPlayerName());
		}
	}
}