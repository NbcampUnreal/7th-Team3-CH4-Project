// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "UI/UW_TitleLayout.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/PRTitlePlayerController.h"

UUW_TitleLayout::UUW_TitleLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_TitleLayout::NativeConstruct()
{
	EnterButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnEnterButtonClicked);
	OptionButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnOptionButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
}

void UUW_TitleLayout::OnEnterButtonClicked()
{
	if (EnterButton) EnterButton->SetIsEnabled(false);
	UGameplayStatics::OpenLevel(GetWorld(), FName("L_Lobby"));
}
void UUW_TitleLayout::OnOptionButtonClicked()
{
}
void UUW_TitleLayout::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}