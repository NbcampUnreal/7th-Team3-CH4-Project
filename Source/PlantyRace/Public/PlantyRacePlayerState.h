// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlantyRacePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PLANTYRACE_API APlantyRacePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	APlantyRacePlayerState();

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
	
};
