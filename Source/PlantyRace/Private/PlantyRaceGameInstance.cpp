// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PlantyRaceGameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

UPlantyRaceGameInstance::UPlantyRaceGameInstance()
{
	GameMapName = TEXT("TestLevel");
}

void UPlantyRaceGameInstance::CreateSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
		this, &UPlantyRaceGameInstance::OnCreateSessionComplete);

	// 방설정
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.NumPublicConnections = 10;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = false;

	SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void UPlantyRaceGameInstance::OnCreateSessionComplete(
	FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(GameMapName + TEXT("?Listen"));
	}
}

void UPlantyRaceGameInstance::FindSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
		this, &UPlantyRaceGameInstance::OnFindSessionsComplete);

	//검색설정
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 10;

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UPlantyRaceGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	if (!SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.Num() > 0)
	{
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
			this, &UPlantyRaceGameInstance::OnJoinSessionComplete);

		SessionInterface->JoinSession(
			0, NAME_GameSession, SessionSearch->SearchResults[0]);
	}
}

void UPlantyRaceGameInstance::OnJoinSessionComplete(
	FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString Address;
		SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

		APlayerController* PC = GetFirstLocalPlayerController();
		if (PC)
		{
			PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UPlantyRaceGameInstance::JoinGameSession()
{
	FindSession();
}

void UPlantyRaceGameInstance::DestroySession()
{
	if (!SessionInterface.IsValid()) return;

	SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
		this, &UPlantyRaceGameInstance::OnDestroySessionComplete);

	SessionInterface->DestroySession(NAME_GameSession);
}

void UPlantyRaceGameInstance::OnDestroySessionComplete(
	FName SessionName, bool bWasSuccessful)
{

}