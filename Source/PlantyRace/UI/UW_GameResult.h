// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_GameResult.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class PLANTYRACE_API UUW_GameResult : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnReturnToTitleButtonClicked();
	void OnReturnToRoomButtonClicked();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UTextBlock> RankingText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UButton> ReturnToTitleButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UButton> ReturnToRoomButton;

};
