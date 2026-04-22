// PRGMB.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/PRGameStateBase.h"
#include "PRGMB.generated.h"

class ASpawnPoint;
class APRPlayerState;
class APlayerController;
class AController;
class APlantyRaceCharacter;
class ACheckPoint;
class APRSoundManager;

UENUM(BlueprintType)
enum class EPRMatchRound : uint8
{
	None		UMETA(DisplayName = "None"),
	Round1		UMETA(DisplayName = "Round 1"),
	Round2		UMETA(DisplayName = "Round 2"),
	Finished	UMETA(DisplayName = "Finished")
};

UCLASS()
class PLANTYRACE_API APRGMB : public AGameModeBase
{
	GENERATED_BODY()

public:
	APRGMB();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<TObjectPtr<ASpawnPoint>> SpawnPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 NextSpawnIndex = 0;

	UPROPERTY()
	TMap<TObjectPtr<AController>, TObjectPtr<ASpawnPoint>> ControllerSpawnPointMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round")
	EPRMatchRound CurrentRound = EPRMatchRound::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	int32 Round1QualifiedCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	int32 FinalRoundWinnerCount = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round")
	TArray<TObjectPtr<APRPlayerState>> FinishOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round")
	TArray<TObjectPtr<APRPlayerState>> QualifiedPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	float FirstPlaceRaceScore = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	float RaceScoreStep = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TSubclassOf<APRSoundManager> SoundManagerClass;

	UPROPERTY()
	TObjectPtr<APRSoundManager> SpawnedSoundManager;

protected:
	void CollectSpawnPoints();
	void SortSpawnPoints();
	float CalculateRaceScoreByRank(int32 Rank) const;
	void UpdateAllPlayerTotalScores();
	void SortPlayersByTotalScore(TArray<TObjectPtr<APRPlayerState>>& Players) const;
	void DisableFinishedPlayer(APlantyRaceCharacter* PlayerCharacter);
	void SetSpectatorViewForFinishedPlayer(APlantyRaceCharacter* FinishedCharacter);
	void SpawnSoundManager();
	void PlayMapBGM();

public:
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	ASpawnPoint* GetSpawnPointByIndex(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RespawnPlayer(APlantyRaceCharacter* PlayerCharacter);

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound1();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound2();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void RegisterPlayerFinish(APlantyRaceCharacter* PlayerCharacter, APRPlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Round")
	void EndCurrentRound();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void ResetRoundData();

	UFUNCTION(BlueprintCallable, Category = "Round")
	bool IsPlayerQualified(APRPlayerState* PlayerState) const;

	UFUNCTION(BlueprintCallable, Category = "Round")
	EPRMatchRound GetCurrentRound() const { return CurrentRound; }

protected:
	void ProcessRound1Results();
	void ProcessRound2Results();
	void PrintFinishOrderLog() const;

protected:
	// 날씨 변경 주기
	UPROPERTY(EditDefaultsOnly, Category = "Weather")
	float WeatherChangeInterval = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weather")
	float WeatherDuration = 15.f;

	// 날씨 변경 타이머 핸들
	FTimerHandle WeatherChangeTimerHandle;
	// 날씨 초기화
	FTimerHandle WeatherResetTimerHandle;

	bool bWeatherActive = false;

	void StartWeatherTimer();

	void ChangeWeatherPeriodically();

	void ResetWeather();

	EWeatherState GetRandomWeather() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	float RoundStartLockDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	float RespawnLockDuration = 2.0f;

	void LockPlayerMovementForDuration(APlantyRaceCharacter* PlayerCharacter, float Duration);
	void LockAllPlayersMovementForDuration(float Duration);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	float RoundTimeLimit = 180.0f;


public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	int32 MinPlayersToStart = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	float LobbyStartDelay = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Result")
	float ResultReturnDelay = 15.0f;

	FTimerHandle LobbyStartTimerHandle;
	FTimerHandle ResultReturnTimerHandle;

	bool bLobbyStartScheduled = false;

	void HandleMapFlowByCurrentMap();
	void TryStartLobbyMatch();
	void StartMatchFromLobby();
	void ReturnToLobbyFromResult();
};