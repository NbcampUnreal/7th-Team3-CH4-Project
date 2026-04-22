// PRPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Types/WeatherEffectTypes.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "PRPlayerState.generated.h"

UCLASS()
class PLANTYRACE_API APRPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	APRPlayerState();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	// 최종 점수
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Score")
	float TotalScore;

	// 골인 점수
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Score")
	float RaceScore;

	// 성장 점수
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Score")
	float GrowthScore;

	// 농작물 성장률
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Player")
	float GrowthRate;

	float MaxGrowthRate = 1000.0f;

	// 탈락 여부
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Player")
	bool bEliminated;

	// 골인 순위
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Race")
	int32 FinishRank;

	UFUNCTION(BlueprintCallable)
	int32 GetFinishRank() const { return FinishRank; }

	// 골인 여부
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Race")
	bool bFinished;

	// 다음 라운드 진출 여부
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Race")
	bool bQualified;

	// 최종 우승 여부
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Race")
	bool bFinalWinner;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FClothesRepData SavedClothesData;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bHasSavedClothesData = false;

	// 라운드 상태 초기화
	UFUNCTION(BlueprintCallable)
	void ResetRoundState();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Lobby")
	bool bIsReady = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Lobby")
	bool bIsHost;

	void SetFinishRank(int32 InRank) { FinishRank = InRank; }

	void SetFinished(bool bInFinished) { bFinished = bInFinished; }
	bool IsFinished() const { return bFinished; }

	void SetQualified(bool bInQualified) { bQualified = bInQualified; }
	bool IsQualified() const { return bQualified; }

	void SetEliminated(bool bInEliminated) { bEliminated = bInEliminated; }
	bool IsEliminated() const { return bEliminated; }

	void SetFinalWinner(bool bInWinner) { bFinalWinner = bInWinner; }
	bool IsFinalWinner() const { return bFinalWinner; }

	void SetRaceScore(float InRaceScore) { RaceScore = InRaceScore; }
	float GetRaceScore() const { return RaceScore; }

	void SetGrowthScore(float InGrowthScore) { GrowthScore = InGrowthScore; }
	float GetGrowthScore() const { return GrowthScore; }

	void SetTotalScore(float InTotalScore) { TotalScore = InTotalScore; }
	float GetTotalScore() const { return TotalScore; }

	UFUNCTION(BlueprintCallable)
	void UpdateGrowthScoreFromRate();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "RPC")
	void ServerRequestOvergrow();

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(BlueprintCallable)
	void SetReady(bool bNewReady);
};