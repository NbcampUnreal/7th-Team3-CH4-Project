// PRGMB.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PRGMB.generated.h"

class ASpawnPoint;
class APRPlayerState;
class APlayerController;
class AController;
class APlantyRaceCharacter;
class ACheckPoint;

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

protected:
	void CollectSpawnPoints();
	void SortSpawnPoints();
	float CalculateRaceScoreByRank(int32 Rank) const;
	void UpdateAllPlayerTotalScores();
	void SortPlayersByTotalScore(TArray<TObjectPtr<APRPlayerState>>& Players) const;
	void DisableFinishedPlayer(APlantyRaceCharacter* PlayerCharacter);
	void SetSpectatorViewForFinishedPlayer(APlantyRaceCharacter* FinishedCharacter);

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
};