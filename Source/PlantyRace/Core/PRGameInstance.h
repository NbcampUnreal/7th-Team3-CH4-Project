// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PRGameInstance.generated.h"

UCLASS()
class PLANTYRACE_API UPRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPRGameInstance();

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	TArray<FString> MapNames;

	UPROPERTY(BlueprintReadWrite, Category = "Map")
	int32 CurrentMapIndex;

	UFUNCTION(BlueprintCallable, Category = "Map")
	FString GetCurrentMapName() const;

	UFUNCTION(BlueprintCallable, Category = "Map")
	bool TravelToMapByIndex(int32 NewMapIndex);

	UFUNCTION(BlueprintCallable, Category = "Map")
	bool TravelToNextMap();

	UFUNCTION(BlueprintCallable, Category = "Map")
	bool TravelToResultMap();

	UFUNCTION(BlueprintCallable, Category = "Map")
	void ResetMapFlow();

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
};