#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PRGMB.generated.h"

class ASpawnPoint;
class APRPlayerState;
class APlayerController;

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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<TObjectPtr<ASpawnPoint>> SpawnPoints;

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

protected:
	void CollectSpawnPoints();
	void SortSpawnPoints();

public:
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	ASpawnPoint* GetSpawnPointByIndex(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound1();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound2();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void RegisterPlayerFinish(APRPlayerState* PlayerState);

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