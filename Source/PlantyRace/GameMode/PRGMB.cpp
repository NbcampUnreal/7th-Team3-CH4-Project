// PRGMB.cpp
#include "GameMode/PRGMB.h"
#include "GameMode/SpawnPoint.h"
#include "GameMode/CheckPoint.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "Core/PRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/PRGameInstance.h"
#include "Core/PRGameStateBase.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Audio/PRSoundManager.h"

APRGMB::APRGMB()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseSeamlessTravel = true;
}

void APRGMB::BeginPlay()
{
	Super::BeginPlay();

	CollectSpawnPoints();
	SpawnSoundManager();
	PlayMapBGM();
	StartRound1();

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (IsValid(GS) == true)
	{
		GS->SetWeather(EWeatherState::Rain);

		StartWeatherTimer();
	}
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

AActor* APRGMB::ChoosePlayerStart_Implementation(AController* Player)
{
	if (SpawnPoints.Num() <= 0)
	{
		CollectSpawnPoints();
	}

	if (SpawnPoints.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChoosePlayerStart] No SpawnPoints found"));
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const int32 SpawnArrayIndex = NextSpawnIndex % SpawnPoints.Num();
	ASpawnPoint* SelectedSpawnPoint = SpawnPoints[SpawnArrayIndex];
	NextSpawnIndex++;

	if (Player && SelectedSpawnPoint)
	{
		ControllerSpawnPointMap.FindOrAdd(Player) = SelectedSpawnPoint;

		UE_LOG(LogTemp, Warning, TEXT("[ChoosePlayerStart] Selected SpawnPoint: %s / SpawnIndex: %d"),
			*SelectedSpawnPoint->GetName(),
			SelectedSpawnPoint->SpawnIndex);

		return SelectedSpawnPoint;
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

APawn* APRGMB::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	APawn* NewPawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);

	if (!NewPawn)
	{
		return nullptr;
	}

	APlantyRaceCharacter* PlayerCharacter = Cast<APlantyRaceCharacter>(NewPawn);
	ASpawnPoint* SpawnPoint = Cast<ASpawnPoint>(StartSpot);

	if (PlayerCharacter && SpawnPoint)
	{
		PlayerCharacter->SetStartSpawnPoint(SpawnPoint);

		if (NewPlayer)
		{
			ControllerSpawnPointMap.FindOrAdd(NewPlayer) = SpawnPoint;
		}

		UE_LOG(LogTemp, Warning, TEXT("[Spawn] StartSpawnPoint set: %s"),
			*SpawnPoint->GetName());
	}

	return NewPawn;
}

void APRGMB::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
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

	for (int32 i = 0; i < SpawnPoints.Num(); ++i)
	{
		if (SpawnPoints[i])
		{
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] SpawnPoints[%d] = %s / SpawnIndex = %d / Location = %s"),
				i,
				*SpawnPoints[i]->GetName(),
				SpawnPoints[i]->SpawnIndex,
				*SpawnPoints[i]->GetActorLocation().ToString());
		}
	}
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

float APRGMB::CalculateRaceScoreByRank(int32 Rank) const
{
	if (Rank <= 0)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, FirstPlaceRaceScore - ((Rank - 1) * RaceScoreStep));
}

void APRGMB::UpdateAllPlayerTotalScores()
{
	if (!GameState)
	{
		return;
	}

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			PRPlayerState->UpdateGrowthScoreFromRate();
		}
	}
}

void APRGMB::SortPlayersByTotalScore(TArray<TObjectPtr<APRPlayerState>>& Players) const
{
	Players.Sort([](const APRPlayerState& A, const APRPlayerState& B)
		{
			return A.GetTotalScore() > B.GetTotalScore();
		});
}

void APRGMB::DisableFinishedPlayer(APlantyRaceCharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return;
	}

	if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	if (AController* Controller = PlayerCharacter->GetController())
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			PlayerCharacter->DisableInput(PC);
		}
	}

	if (UCapsuleComponent* Capsule = PlayerCharacter->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	PlayerCharacter->SetActorHiddenInGame(true);
	PlayerCharacter->SetActorEnableCollision(false);
}

void APRGMB::SetSpectatorViewForFinishedPlayer(APlantyRaceCharacter* FinishedCharacter)
{
	if (!FinishedCharacter)
	{
		return;
	}

	APlayerController* FinishedPC = Cast<APlayerController>(FinishedCharacter->GetController());
	if (!FinishedPC)
	{
		return;
	}

	for (TActorIterator<APlantyRaceCharacter> It(GetWorld()); It; ++It)
	{
		APlantyRaceCharacter* OtherCharacter = *It;
		if (!OtherCharacter || OtherCharacter == FinishedCharacter)
		{
			continue;
		}

		if (OtherCharacter->IsHidden())
		{
			continue;
		}

		APRPlayerState* OtherPS = OtherCharacter->GetPlayerState<APRPlayerState>();
		if (!OtherPS)
		{
			continue;
		}

		if (OtherPS->IsFinished())
		{
			continue;
		}

		FinishedPC->SetViewTargetWithBlend(OtherCharacter, 0.5f);
		return;
	}
}

void APRGMB::SpawnSoundManager()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!SoundManagerClass)
	{
		return;
	}

	if (IsValid(SpawnedSoundManager))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	SpawnedSoundManager = GetWorld()->SpawnActor<APRSoundManager>(
		SoundManagerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	APRGameStateBase* PRGameState = GetGameState<APRGameStateBase>();
	if (IsValid(PRGameState))
	{
		PRGameState->SetSoundManager(SpawnedSoundManager);
	}
}

void APRGMB::PlayMapBGM()
{
	APRGameStateBase* PRGameState = GetGameState<APRGameStateBase>();
	if (!IsValid(PRGameState))
	{
		return;
	}

	APRSoundManager* SoundManager = PRGameState->GetSoundManager();
	if (!IsValid(SoundManager))
	{
		return;
	}

	const FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);

	if (MapName == TEXT("L_Title"))
	{
		SoundManager->PlayBGMByType(EPRBGMType::Title);
	}
	else if (MapName == TEXT("L_Lobby"))
	{
		SoundManager->PlayBGMByType(EPRBGMType::Lobby);
	}
	else if (MapName == TEXT("L_Round1"))
	{
		SoundManager->PlayBGMByType(EPRBGMType::Round1);
	}
	else if (MapName == TEXT("L_Round2"))
	{
		SoundManager->PlayBGMByType(EPRBGMType::Round2);
	}
	else if (MapName == TEXT("L_Result"))
	{
		SoundManager->PlayBGMByType(EPRBGMType::Result);
	}
}

void APRGMB::RespawnPlayer(APlantyRaceCharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return;
	}

	if (ACheckPoint* LastCheckpoint = PlayerCharacter->GetLastCheckpoint())
	{
		PlayerCharacter->SetActorLocationAndRotation(
			LastCheckpoint->GetRespawnLocation(),
			LastCheckpoint->GetRespawnRotation()
		);

		APRGameStateBase* GS = GetGameState<APRGameStateBase>();
		if (IsValid(GS))
		{
			if (APRSoundManager* SM = GS->GetSoundManager())
			{
				SM->PlayRespawnSFX(PlayerCharacter->GetActorLocation());
			}
		}

		LockPlayerMovementForDuration(PlayerCharacter, RespawnLockDuration);

		UE_LOG(LogTemp, Warning, TEXT("[Respawn] Player moved to LastCheckpoint"));
		return;
	}

	if (AController* Controller = PlayerCharacter->GetController())
	{
		if (TObjectPtr<ASpawnPoint>* FoundSpawn = ControllerSpawnPointMap.Find(Controller))
		{
			if (*FoundSpawn)
			{
				PlayerCharacter->SetActorLocationAndRotation(
					(*FoundSpawn)->GetActorLocation(),
					(*FoundSpawn)->GetActorRotation()
				);

				APRGameStateBase* GS = GetGameState<APRGameStateBase>();
				if (IsValid(GS))
				{
					if (APRSoundManager* SM = GS->GetSoundManager())
					{
						SM->PlayRespawnSFX(PlayerCharacter->GetActorLocation());
					}
				}

				LockPlayerMovementForDuration(PlayerCharacter, RespawnLockDuration);

				UE_LOG(LogTemp, Warning, TEXT("[Respawn] Player moved to StartSpawnPoint (ControllerMap)"));
				return;
			}
		}
	}

	if (ASpawnPoint* StartSpawn = PlayerCharacter->GetStartSpawnPoint())
	{
		PlayerCharacter->SetActorLocationAndRotation(
			StartSpawn->GetActorLocation(),
			StartSpawn->GetActorRotation()
		);

		APRGameStateBase* GS = GetGameState<APRGameStateBase>();
		if (IsValid(GS))
		{
			if (APRSoundManager* SM = GS->GetSoundManager())
			{
				SM->PlayRespawnSFX(PlayerCharacter->GetActorLocation());
			}
		}

		LockPlayerMovementForDuration(PlayerCharacter, RespawnLockDuration);

		UE_LOG(LogTemp, Warning, TEXT("[Respawn] Player moved to StartSpawnPoint (Character)"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Respawn] No valid respawn point"));
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

	APRGameStateBase* PRGameState = GetGameState<APRGameStateBase>();
	if (IsValid(PRGameState))
	{
		if (APRSoundManager* SoundManager = PRGameState->GetSoundManager())
		{
			SoundManager->PlayRoundStartSFX();
		}
	}
	LockAllPlayersMovementForDuration(RoundStartLockDuration);
	GetWorldTimerManager().SetTimer(
		RoundTimerHandle,
		this,
		&APRGMB::EndCurrentRound,
		RoundTimeLimit,
		false
	);
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

			AController* Controller = BasePlayerState->GetOwner<AController>();
			if (Controller)
			{
				APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(Controller->GetPawn());
				if (Character && !bQualified)
				{
					DisableFinishedPlayer(Character);
				}
			}
		}
	}

	APRGameStateBase* PRGameState = GetGameState<APRGameStateBase>();
	if (IsValid(PRGameState))
	{
		if (APRSoundManager* SoundManager = PRGameState->GetSoundManager())
		{
			SoundManager->PlayRoundStartSFX();
		}
	}
	LockAllPlayersMovementForDuration(RoundStartLockDuration);
	GetWorldTimerManager().SetTimer(
		RoundTimerHandle,
		this,
		&APRGMB::EndCurrentRound,
		RoundTimeLimit,
		false
	);
}

void APRGMB::RegisterPlayerFinish(APlantyRaceCharacter* PlayerCharacter, APRPlayerState* PlayerState)
{
	if (!PlayerCharacter || !PlayerState)
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
	PlayerState->SetRaceScore(CalculateRaceScoreByRank(NewRank));
	PlayerState->SetTotalScore(PlayerState->GetRaceScore() + PlayerState->GetGrowthScore());

	FinishOrder.Add(PlayerState);

	DisableFinishedPlayer(PlayerCharacter);
	SetSpectatorViewForFinishedPlayer(PlayerCharacter);

	UE_LOG(LogTemp, Warning, TEXT("[Finish] %s finished with rank %d / RaceScore %.1f / TotalScore %.1f"),
		*PlayerState->GetPlayerName(),
		NewRank,
		PlayerState->GetRaceScore(),
		PlayerState->GetTotalScore());

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
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
	UPRGameInstance* GI = GetGameInstance<UPRGameInstance>();

	if (CurrentRound == EPRMatchRound::Round1)
	{
		UE_LOG(LogTemp, Warning, TEXT("========== ROUND 1 END =========="));
		ProcessRound1Results();

		if (GI)
		{
			GI->TravelToMapByIndex(3); // L_Round2
		}
	}
	else if (CurrentRound == EPRMatchRound::Round2)
	{
		UE_LOG(LogTemp, Warning, TEXT("========== ROUND 2 END =========="));
		ProcessRound2Results();

		CurrentRound = EPRMatchRound::Finished;

		APRGameStateBase* PRGameState = GetGameState<APRGameStateBase>();
		if (IsValid(PRGameState))
		{
			if (APRSoundManager* SoundManager = PRGameState->GetSoundManager())
			{
				SoundManager->PlayVictorySFX();
			}
		}

		if (GI)
		{
			GI->TravelToMapByIndex(4); // L_Result
		}

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

	if (!GameState)
	{
		return;
	}

	UpdateAllPlayerTotalScores();

	TArray<TObjectPtr<APRPlayerState>> AllPlayers;
	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			AllPlayers.Add(PRPlayerState);
		}
	}

	SortPlayersByTotalScore(AllPlayers);

	for (int32 i = 0; i < AllPlayers.Num(); ++i)
	{
		if (APRPlayerState* PRPlayerState = AllPlayers[i])
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

	UE_LOG(LogTemp, Warning, TEXT("----- ROUND 1 RESULT (TotalScore) -----"));
	for (int32 i = 0; i < AllPlayers.Num(); ++i)
	{
		if (APRPlayerState* PRPlayerState = AllPlayers[i])
		{
			UE_LOG(LogTemp, Warning, TEXT("%d. %s / RaceScore %.1f / GrowthScore %.1f / TotalScore %.1f / Qualified %s"),
				i + 1,
				*PRPlayerState->GetPlayerName(),
				PRPlayerState->GetRaceScore(),
				PRPlayerState->GetGrowthScore(),
				PRPlayerState->GetTotalScore(),
				PRPlayerState->IsQualified() ? TEXT("YES") : TEXT("NO"));
		}
	}
}

void APRGMB::ProcessRound2Results()
{
	if (!GameState)
	{
		return;
	}

	UpdateAllPlayerTotalScores();

	TArray<TObjectPtr<APRPlayerState>> FinalPlayers;
	for (APRPlayerState* QualifiedPlayer : QualifiedPlayers)
	{
		if (QualifiedPlayer)
		{
			QualifiedPlayer->SetFinalWinner(false);
			FinalPlayers.Add(QualifiedPlayer);
		}
	}

	SortPlayersByTotalScore(FinalPlayers);

	if (FinalPlayers.Num() > 0)
	{
		if (APRPlayerState* Winner = FinalPlayers[0])
		{
			Winner->SetFinalWinner(true);

			UE_LOG(LogTemp, Warning, TEXT("===== FINAL WINNER: %s / RaceScore %.1f / GrowthScore %.1f / TotalScore %.1f ====="),
				*Winner->GetPlayerName(),
				Winner->GetRaceScore(),
				Winner->GetGrowthScore(),
				Winner->GetTotalScore());
		}
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
			UE_LOG(LogTemp, Warning, TEXT("%d. %s / RaceScore %.1f / GrowthScore %.1f / TotalScore %.1f"),
				i + 1,
				*PRPlayerState->GetPlayerName(),
				PRPlayerState->GetRaceScore(),
				PRPlayerState->GetGrowthScore(),
				PRPlayerState->GetTotalScore());
		}
	}
}

void APRGMB::StartWeatherTimer()
{
	GetWorldTimerManager().SetTimer(
		WeatherChangeTimerHandle,
		this,
		&APRGMB::ChangeWeatherPeriodically,
		WeatherChangeInterval,
		true
	);
}

void APRGMB::ChangeWeatherPeriodically()
{
	if (bWeatherActive == true)
	{
		return;
	}

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (IsValid(GS) == true)
	{
		EWeatherState NewWeather = GetRandomWeather();

		GS->SetWeather(NewWeather);

		if (NewWeather != EWeatherState::None)
		{
			bWeatherActive = true;

			GetWorldTimerManager().SetTimer(
				WeatherResetTimerHandle,
				this,
				&APRGMB::ResetWeather,
				WeatherDuration,
				false
			);
		}
	}
}

void APRGMB::ResetWeather()
{
	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (IsValid(GS) == true)
	{
		GS->SetWeather(EWeatherState::None);

		bWeatherActive = false;
	}
}

EWeatherState APRGMB::GetRandomWeather() const
{
	int32 ChangeRoll = FMath::RandRange(0, 1);

	if (ChangeRoll != 0)
	{
		return EWeatherState::None;
	}

	int32 WeatherRoll = FMath::RandRange(0, 2);

	if (WeatherRoll == 0)
	{
		return EWeatherState::Rain;
	}
	else if (WeatherRoll == 1)
	{
		return EWeatherState::Sun;
	}

	return EWeatherState::Tornado;
}

void APRGMB::LockPlayerMovementForDuration(APlantyRaceCharacter* PlayerCharacter, float Duration)
{
	if (!PlayerCharacter)
	{
		return;
	}

	if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		[PlayerCharacter]()
		{
			if (!IsValid(PlayerCharacter))
			{
				return;
			}

			if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Walking);
			}
		},
		Duration,
		false
	);
}

void APRGMB::LockAllPlayersMovementForDuration(float Duration)
{
	if (!GameState)
	{
		return;
	}

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (!BasePlayerState)
		{
			continue;
		}

		AController* Controller = Cast<AController>(BasePlayerState->GetOwner());
		if (!Controller)
		{
			continue;
		}

		APlantyRaceCharacter* PlayerCharacter = Cast<APlantyRaceCharacter>(Controller->GetPawn());
		if (!PlayerCharacter)
		{
			continue;
		}

		LockPlayerMovementForDuration(PlayerCharacter, Duration);
	}
}