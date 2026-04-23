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

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class APlantyRacePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

public:
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