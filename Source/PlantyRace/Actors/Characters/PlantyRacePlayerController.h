// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlantyRacePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UPRWeatherWidget;
class UUW_GameResult;
class UInputAction;
class APlantyRaceCharacter;

UCLASS(abstract)
class APlantyRacePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	TObjectPtr<UUserWidget> MobileControlsWidget;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	virtual void PlayerTick(float DeltaTime) override;

	void CreateWeatherWidget();

	UFUNCTION()
	void UpdateWeatherUI();

	UFUNCTION()
	void HandleWeatherChanged();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	UUserWidget* HUDWidgetInstance;

	UFUNCTION(BlueprintPure, Category = "HUD")
	UUserWidget* GetHUDWidget() const;

	UFUNCTION(Client, Reliable)
	void ClientRPCShowGameResultWidget(int32 InRanking);

	void UpdateHUD();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUW_GameResult> GameResultUIClass;

	UFUNCTION(Client, Reliable)
	void ClientPlayCheckPointSFX(FVector Location);

	UFUNCTION(Client, Reliable)
	void ClientPlayRespawnSFX(FVector Location);

	UFUNCTION(Client, Reliable)
	void ClientPlayFinishSFX(FVector Location);

	UFUNCTION(Client, Reliable)
	void ClientPlayMapBGM(EPRBGMType BGMType);

	UFUNCTION(Client, Reliable)
	void ClientPlayRoundStartSFX();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Spectate")
	TObjectPtr<UInputAction> SpectatePrevAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Spectate")
	TObjectPtr<UInputAction> SpectateNextAction;

	UFUNCTION(BlueprintCallable, Category = "Spectate")
	bool CanSpectate() const;

	UFUNCTION(BlueprintCallable, Category = "Spectate")
	void StartSpectatingOtherPlayers();

	UFUNCTION(BlueprintCallable, Category = "Spectate")
	void RefreshSpectateTargets();

	UFUNCTION(BlueprintCallable, Category = "Spectate")
	void SpectatePrevPlayer();

	UFUNCTION(BlueprintCallable, Category = "Spectate")
	void SpectateNextPlayer();


protected:
	UPROPERTY()
	UPRWeatherWidget* WeatherWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UPRWeatherWidget> WeatherWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<APlantyRaceCharacter>> SpectateTargets;

	UPROPERTY()
	int32 CurrentSpectateIndex = INDEX_NONE;

	void ApplySpectateTargetByIndex(int32 TargetIndex);
	bool IsValidSpectateTarget(APlantyRaceCharacter* TargetCharacter) const;
};