// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PlantyRaceGameInstance.generated.h"

/**
 *
 */
UCLASS()
class PLANTYRACE_API UPlantyRaceGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPlantyRaceGameInstance();

	// 방
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSession();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinGameSession();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroySession();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Session")
	FString GameMapName;

private:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

};
