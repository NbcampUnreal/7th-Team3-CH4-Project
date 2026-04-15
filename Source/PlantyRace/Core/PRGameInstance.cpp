// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "PRGameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

UPRGameInstance::UPRGameInstance()
{
	GameMapName = TEXT("TestLevel");
	CurrentMapIndex = 0;

	MapNames.Add(TEXT("L_Title"));
	MapNames.Add(TEXT("L_Lobby"));
	MapNames.Add(TEXT("L_Round1"));
	MapNames.Add(TEXT("L_Round2"));
	MapNames.Add(TEXT("L_Result"));
}

void UPRGameInstance::CreateSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
	SessionInterface->OnCreateSessionCompleteDelegates.Clear();
	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
		this, &UPRGameInstance::OnCreateSessionComplete);

	// 방설정
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.NumPublicConnections = 10;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = false;

	const int32 LocalPlayerIndex = 0;
	if (LocalPlayers.Num() > 0)
	{
		SessionInterface->CreateSession(
			*LocalPlayers[LocalPlayerIndex]->GetPreferredUniqueNetId(),
			NAME_GameSession, SessionSettings);
	}
	else
	{
		SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
	}

	SessionSettings.Set(FName("SESSION_NAME"), FString(TEXT("PlantyRace_Room")), EOnlineDataAdvertisementType::ViaOnlineService);
}

void UPRGameInstance::OnCreateSessionComplete(
	FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(GameMapName + TEXT("?Listen"));
	}
}

void UPRGameInstance::FindSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	SessionInterface->OnFindSessionsCompleteDelegates.Clear();
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
		this, &UPRGameInstance::OnFindSessionsComplete);

	//검색설정
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 10;

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UPRGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	if (!SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.Num() > 0)
	{
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
			this, &UPRGameInstance::OnJoinSessionComplete);

		SessionInterface->JoinSession(
			0, NAME_GameSession, SessionSearch->SearchResults[0]);
	}
}

void UPRGameInstance::OnJoinSessionComplete(
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

void UPRGameInstance::JoinGameSession()
{
	FindSession();
}

void UPRGameInstance::DestroySession()
{
	if (!SessionInterface.IsValid()) return;

	SessionInterface->OnDestroySessionCompleteDelegates.Clear();
	SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
		this, &UPRGameInstance::OnDestroySessionComplete);

	SessionInterface->DestroySession(NAME_GameSession);
}

void UPRGameInstance::OnDestroySessionComplete(
	FName SessionName, bool bWasSuccessful)
{

}

TArray<FString> UPRGameInstance::GetFoundSessionNames()
{
	TArray<FString> Names;
	if (!SessionSearch.IsValid()) return Names;

	for (auto& Result : SessionSearch->SearchResults)
	{
		// 방 이름 가져오기
		FString SessionName;
		Result.Session.SessionSettings.Get(
			FName("SESSION_NAME"), SessionName);
		Names.Add(SessionName);
	}
	return Names;
}

void UPRGameInstance::JoinSessionByIndex(int32 Index)
{
	if (!SessionSearch.IsValid()) return;
	if (!SessionSearch->SearchResults.IsValidIndex(Index)) return;

	SessionInterface->OnJoinSessionCompleteDelegates.Clear();
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
		this, &UPRGameInstance::OnJoinSessionComplete);

	SessionInterface->JoinSession(
		0, NAME_GameSession, SessionSearch->SearchResults[Index]);
}