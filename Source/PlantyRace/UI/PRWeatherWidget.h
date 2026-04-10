// Copyright © 2026 33Fellowship. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRWeatherWidget.generated.h"

class UTextBlock;

UCLASS()
class PLANTYRACE_API UPRWeatherWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void RefreshWeatherText();

	UFUNCTION(BlueprintCallable)
	void SetWeatherText(const FText& InText);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeatherText;
};
