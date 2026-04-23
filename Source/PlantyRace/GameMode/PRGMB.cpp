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
#include "Actors/Characters/PlantyRacePlayerController.h"
#include "Audio/PRSoundManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// 기본 / 생명주기
////////////////////////////////////////////////////////////////////////////////////////////////////

APRGMB::APRGMB()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseSeamlessTravel = true;
}

void APRGMB::BeginPlay()
{
	Super::BeginPlay();

	SpawnSoundManager();
	HandleMapFlowByCurrentMap();

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (IsValid(GS))
	{
		GS->SetWeather(EWeatherState::Sun);
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

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (IsValid(GS))
	{
		GS->CheackAllPlayersReady();
	}

	const FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (MapName == TEXT("L_Lobby"))
	{
		TryStartLobbyMatch();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 스폰 관련
////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// 점수 / 결과 계산
////////////////////////////////////////////////////////////////////////////////////////////////////

void APRGMB::PrintFinishOrderLog() const
{
	UE_LOG(LogTemp, Warning, TEXT("----- Current Finish Order -----"));

	for (int32 i = 0; i < FinishOrder.Num(); ++i)
	{
		const APRPlayerState* PRPlayerState = FinishOrder[i];
		if (PRPlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d. %s"),
				i + 1,
				*PRPlayerState->GetPlayerName());
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// 플레이어 상태 / 관전 / 이동 잠금
////////////////////////////////////////////////////////////////////////////////////////////////////

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

	APlantyRacePlayerController* FinishedPC = Cast<APlantyRacePlayerController>(FinishedCharacter->GetController());
	if (!FinishedPC)
	{
		return;
	}

	FinishedPC->StartSpectatingOtherPlayers();
}
void APRGMB::LockPlayerMovementForDuration(APlantyRaceCharacter* PlayerCharacter, float Duration)
{
	if (!PlayerCharacter)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());

	if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	if (PC)
	{
		PlayerCharacter->DisableInput(PC);
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
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

			APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());

			if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Walking);
			}

			if (PC)
			{
				PlayerCharacter->EnableInput(PC);
				PC->SetIgnoreMoveInput(false);
				PC->SetIgnoreLookInput(false);
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// 사운드 매니저 / BGM
////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// 리스폰
////////////////////////////////////////////////////////////////////////////////////////////////////

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
		PlayerCharacter->SetActionState(EPlayerActionState::Idle);

		if (APlantyRacePlayerController* PRPC = Cast<APlantyRacePlayerController>(PlayerCharacter->GetController()))
		{
			PRPC->ClientPlayRespawnSFX(PlayerCharacter->GetActorLocation());
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

				PlayerCharacter->SetActionState(EPlayerActionState::Idle);

				if (APlantyRacePlayerController* PRPC = Cast<APlantyRacePlayerController>(PlayerCharacter->GetController()))
				{
					PRPC->ClientPlayRespawnSFX(PlayerCharacter->GetActorLocation());
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

		PlayerCharacter->SetActionState(EPlayerActionState::Idle);

		if (APlantyRacePlayerController* PRPC = Cast<APlantyRacePlayerController>(PlayerCharacter->GetController()))
		{
			PRPC->ClientPlayRespawnSFX(PlayerCharacter->GetActorLocation());
		}

		UE_LOG(LogTemp, Warning, TEXT("[Respawn] Player moved to StartSpawnPoint (Character)"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Respawn] No valid respawn point"));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// 라운드 시작 / 종료 / 타이머
////////////////////////////////////////////////////////////////////////////////////////////////////

void APRGMB::StartRound1()
{
	ResetRoundData();
	QualifiedPlayers.Empty();

	CurrentRound = EPRMatchRound::Round1;

	if (!GameState)
	{
		return;
	}

	const int32 PlayerCount = GameState->PlayerArray.Num();
	Round1QualifiedCount = FMath::Max(1, FMath::CeilToInt(PlayerCount / 2.0f));

	UE_LOG(LogTemp, Warning, TEXT("========== ROUND 1 START =========="));
	UE_LOG(LogTemp, Warning, TEXT("Rule: First %d players qualify."), Round1QualifiedCount);

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
		PRGameState->SetRoundNumber(1);
		PRGameState->SetRemainingTime(RoundTimeLimit);
	}

	LockAllPlayersMovementForDuration(RoundStartLockDuration);

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (!BasePlayerState)
		{
			continue;
		}

		APlantyRacePlayerController* PRPC = Cast<APlantyRacePlayerController>(BasePlayerState->GetOwner());
		if (PRPC)
		{
			PRPC->ClientPlayRoundStartSFX();
		}
	}

	FTimerHandle RoundStartDelayHandle;
	GetWorldTimerManager().SetTimer(
		RoundStartDelayHandle,
		[this]()
		{
			StartRoundTimer(RoundTimeLimit);
		},
		RoundStartLockDuration,
		false
	);
}

void APRGMB::StartRound2()
{
	ResetRoundData();
	CurrentRound = EPRMatchRound::Round2;
	QualifiedPlayers.Empty();

	if (!GameState)
	{
		return;
	}

	UPRGameInstance* GI = GetGameInstance<UPRGameInstance>();

	UE_LOG(LogTemp, Warning, TEXT("========== ROUND 2 START =========="));

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState);
		if (!PRPlayerState)
		{
			continue;
		}

		const bool bQualifiedFromGI =
			(GI && GI->IsPlayerQualifiedForRound2(PRPlayerState->GetPlayerName()));

		PRPlayerState->ResetRoundState();
		PRPlayerState->SetQualified(bQualifiedFromGI);
		PRPlayerState->SetEliminated(!bQualifiedFromGI);
		PRPlayerState->SetFinalWinner(false);

		if (bQualifiedFromGI)
		{
			QualifiedPlayers.Add(PRPlayerState);
		}

		AController* Controller = Cast<AController>(BasePlayerState->GetOwner());
		if (Controller)
		{
			APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(Controller->GetPawn());
			if (Character)
			{
				if (!bQualifiedFromGI)
				{
					DisableFinishedPlayer(Character);
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[Round2 Init] %s / Qualified: %s"),
			*PRPlayerState->GetPlayerName(),
			bQualifiedFromGI ? TEXT("YES") : TEXT("NO"));
	}

	APRGameStateBase* PRGameState = GetGameState<APRGameStateBase>();
	if (IsValid(PRGameState))
	{
		PRGameState->SetRoundNumber(2);
		PRGameState->SetRemainingTime(RoundTimeLimit);
	}

	LockAllPlayersMovementForDuration(RoundStartLockDuration);

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (!BasePlayerState)
		{
			continue;
		}

		APlantyRacePlayerController* PRPC = Cast<APlantyRacePlayerController>(BasePlayerState->GetOwner());
		if (PRPC)
		{
			PRPC->ClientPlayRoundStartSFX();
		}
	}

	FTimerHandle RoundStartDelayHandle;
	GetWorldTimerManager().SetTimer(
		RoundStartDelayHandle,
		[this]()
		{
			StartRoundTimer(RoundTimeLimit);
		},
		RoundStartLockDuration,
		false
	);
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
			TArray<FString> QualifiedNames;

			for (APRPlayerState* QualifiedPS : QualifiedPlayers)
			{
				if (IsValid(QualifiedPS))
				{
					QualifiedNames.Add(QualifiedPS->GetPlayerName());
				}
			}

			GI->SaveQualifiedPlayers(QualifiedNames);

			if (QualifiedPlayers.Num() > 0)
			{
				GI->TravelToMapByIndex(3); // L_Round2
			}
			else
			{
				GI->ClearQualifiedPlayers();
				GI->TravelToMapByIndex(1); // L_Lobby
			}
		}
	}
	else if (CurrentRound == EPRMatchRound::Round2)
	{
		UE_LOG(LogTemp, Warning, TEXT("========== ROUND 2 END =========="));
		ProcessRound2Results();

		CurrentRound = EPRMatchRound::Finished;

		if (GI)
		{
			GI->TravelToMapByIndex(4); // L_Result
		}

		UE_LOG(LogTemp, Warning, TEXT("========== MATCH FINISHED =========="));
	}
}

void APRGMB::StartRoundTimer(float InTime)
{
	if (!HasAuthority())
	{
		return;
	}

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (!IsValid(GS))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RoundTimerHandle);

	GS->SetRemainingTime(InTime);

	GetWorldTimerManager().SetTimer(
		RoundTimerHandle,
		this,
		&APRGMB::UpdateRoundTimer,
		1.0f,
		true
	);
}

void APRGMB::UpdateRoundTimer()
{
	if (!HasAuthority())
	{
		return;
	}

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (!IsValid(GS))
	{
		return;
	}

	const float NewTime = FMath::Max(0.0f, GS->RemainingTime - 1.0f);
	GS->SetRemainingTime(NewTime);

	if (NewTime <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(RoundTimerHandle);
		EndCurrentRound();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 라운드 결과 처리
////////////////////////////////////////////////////////////////////////////////////////////////////

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

	if (APlantyRacePlayerController* PRPC = Cast<APlantyRacePlayerController>(PlayerCharacter->GetController()))
	{
		PRPC->ClientPlayFinishSFX(PlayerCharacter->GetActorLocation());
	}

	FinishOrder.Add(PlayerState);

	DisableFinishedPlayer(PlayerCharacter);
	SetSpectatorViewForFinishedPlayer(PlayerCharacter);

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
}void APRGMB::ResetRoundData()
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

	const int32 MaxQualifiedCount = FMath::Min(Round1QualifiedCount, FinishOrder.Num());

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			PRPlayerState->SetQualified(false);
			PRPlayerState->SetEliminated(true);
		}
	}

	for (int32 i = 0; i < MaxQualifiedCount; ++i)
	{
		if (APRPlayerState* PRPlayerState = FinishOrder[i])
		{
			PRPlayerState->SetQualified(true);
			PRPlayerState->SetEliminated(false);
			QualifiedPlayers.Add(PRPlayerState);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("----- ROUND 1 RESULT -----"));
	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s / FinishRank: %d / Qualified: %s"),
				*PRPlayerState->GetPlayerName(),
				PRPlayerState->GetFinishRank(),
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

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		if (APRPlayerState* PRPlayerState = Cast<APRPlayerState>(BasePlayerState))
		{
			PRPlayerState->SetFinalWinner(false);
		}
	}

	if (FinishOrder.Num() > 0)
	{
		if (APRPlayerState* Winner = FinishOrder[0])
		{
			Winner->SetFinalWinner(true);

			UE_LOG(LogTemp, Warning, TEXT("===== FINAL WINNER: %s ====="),
				*Winner->GetPlayerName());
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// 날씨 시스템
////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// 맵 흐름 / 로비 시작 / 결과 복귀
////////////////////////////////////////////////////////////////////////////////////////////////////

void APRGMB::HandleMapFlowByCurrentMap()
{
	const FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);

	if (MapName == TEXT("L_Title"))
	{
		PlayMapBGM();
		return;
	}

	if (MapName == TEXT("L_Lobby"))
	{
		CurrentRound = EPRMatchRound::None;
		bLobbyStartScheduled = false;

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}

		PlayMapBGM();
		TryStartLobbyMatch();
		return;
	}

	if (MapName == TEXT("L_Round1"))
	{
		CollectSpawnPoints();
		PlayMapBGM();

		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(
			DelayHandle,
			[this]()
			{
				StartRound1();
			},
			0.5f,
			false
		);
		return;
	}

	if (MapName == TEXT("L_Round2"))
	{
		CollectSpawnPoints();
		PlayMapBGM();

		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(
			DelayHandle,
			[this]()
			{
				StartRound2();
			},
			0.5f,
			false
		);
		return;
	}

	if (MapName == TEXT("L_Result"))
	{
		CurrentRound = EPRMatchRound::Finished;
		PlayMapBGM();

		GetWorldTimerManager().SetTimer(
			ResultReturnTimerHandle,
			this,
			&APRGMB::ReturnToLobbyFromResult,
			ResultReturnDelay,
			false
		);
	}
}
void APRGMB::TryStartLobbyMatch()
{
	if (bLobbyStartScheduled)
	{
		return;
	}

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (!IsValid(GS))
	{
		return;
	}

	const int32 PlayerCount = GS->PlayerArray.Num();
	if (PlayerCount < MinPlayersToStart)
	{
		return;
	}

	if (!GS->bAllPlayersReady)
	{
		return;
	}

	bLobbyStartScheduled = true;

	GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);

	GS->SetRemainingTime(LobbyStartDelay);

	GetWorldTimerManager().SetTimer(
		LobbyStartTimerHandle,
		this,
		&APRGMB::UpdateLobbyCountdown,
		1.0f,
		true
	);

	UE_LOG(LogTemp, Warning, TEXT("[TryStartLobbyMatch] PlayerCount: %d / MinPlayersToStart: %d / AllReady: %d / Scheduled: %d"),
		PlayerCount,
		MinPlayersToStart,
		GS->bAllPlayersReady,
		bLobbyStartScheduled);
}
void APRGMB::StartMatchFromLobby()
{
	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (!IsValid(GS))
	{
		bLobbyStartScheduled = false;
		GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
		return;
	}

	if (GS->PlayerArray.Num() < MinPlayersToStart || !GS->bAllPlayersReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Lobby] Start canceled at final check / PlayerCount: %d / MinPlayers: %d / AllReady: %d"),
			GS->PlayerArray.Num(),
			MinPlayersToStart,
			GS->bAllPlayersReady);

		bLobbyStartScheduled = false;
		GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
		GS->SetRemainingTime(0.0f);
		return;
	}

	UPRGameInstance* GI = GetGameInstance<UPRGameInstance>();
	if (!GI)
	{
		bLobbyStartScheduled = false;
		GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
		return;
	}

	bLobbyStartScheduled = false;
	GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
	GS->SetRemainingTime(0.0f);

	GI->ClearQualifiedPlayers();
	GI->TravelToMapByIndex(2);
}
void APRGMB::ReturnToLobbyFromResult()
{
	UPRGameInstance* GI = GetGameInstance<UPRGameInstance>();
	if (!GI)
	{
		return;
	}

	GI->ClearQualifiedPlayers();
	GI->TravelToMapByIndex(1); // L_Lobby
}

void APRGMB::CancelLobbyMatchStart()
{
	if (!bLobbyStartScheduled)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
	bLobbyStartScheduled = false;

	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (IsValid(GS))
	{
		GS->SetRemainingTime(0.0f);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Lobby] Match start canceled"));
}
void APRGMB::UpdateLobbyCountdown()
{
	APRGameStateBase* GS = GetGameState<APRGameStateBase>();
	if (!IsValid(GS))
	{
		GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
		bLobbyStartScheduled = false;
		return;
	}

	if (GS->PlayerArray.Num() < MinPlayersToStart || !GS->bAllPlayersReady)
	{
		GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
		bLobbyStartScheduled = false;
		GS->SetRemainingTime(0.0f);
		return;
	}

	const float NewTime = FMath::Max(0.0f, GS->RemainingTime - 1.0f);
	GS->SetRemainingTime(NewTime);

	if (NewTime <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(LobbyStartTimerHandle);
		StartMatchFromLobby();
	}
}