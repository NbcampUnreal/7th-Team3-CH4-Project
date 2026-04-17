// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

class UTextBlock;

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_GameResult.generated.h"

/**
 * 
 */
UCLASS()
class PLANTYRACE_API UUW_GameResult : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UTextBlock> RankingText;

};
