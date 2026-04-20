// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "UW_GameResult.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UUW_GameResult::NativeConstruct()
{
	Super::NativeConstruct();

	if (false == ReturnToTitleButton.Get()->OnClicked.IsAlreadyBound(this, &ThisClass::OnReturnToTitleButtonClicked))
	{
		ReturnToTitleButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnReturnToTitleButtonClicked);
	}

	if (false == ReturnToRoomButton.Get()->OnClicked.IsAlreadyBound(this, &ThisClass::OnReturnToRoomButtonClicked))
	{
		ReturnToRoomButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnReturnToRoomButtonClicked);
	}
}

void UUW_GameResult::OnReturnToTitleButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Title")), true);

}

void UUW_GameResult::OnReturnToRoomButtonClicked()
{
	//UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Title")), true);
}