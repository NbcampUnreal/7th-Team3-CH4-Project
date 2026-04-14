#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Types/WeatherEffectTypes.h"
#include "PRPlayerState.generated.h"

UCLASS()
class PLANTYRACE_API APRPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	APRPlayerState();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 순위 계산 점수
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Player")
	float PlayerScore;

	// 농작물 성장률
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Player")
	float GrowthRate;

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

	// 라운드 상태 초기화
	UFUNCTION(BlueprintCallable)
	void ResetRoundState();

	void SetFinishRank(int32 InRank) { FinishRank = InRank; }

	void SetFinished(bool bInFinished) { bFinished = bInFinished; }
	bool IsFinished() const { return bFinished; }

	void SetQualified(bool bInQualified) { bQualified = bInQualified; }

	void SetEliminated(bool bInEliminated) { bEliminated = bInEliminated; }

	void SetFinalWinner(bool bInWinner) { bFinalWinner = bInWinner; }

};