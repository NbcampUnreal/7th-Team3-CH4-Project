// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "Controller/PRTitlePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void APRTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	if (IsValid(UIWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}
}

void APRTitlePlayerController::JoinServer(const FString& InIPAddress)
{
	FName NextLevelName = FName(*InIPAddress);
	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true);
}
