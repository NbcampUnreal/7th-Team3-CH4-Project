// Copyright © 2026 33Fellowship. All Rights Reserved.

#include "PRGameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UPRGameInstance::UPRGameInstance()
{
	GameMapName = TEXT("L_Round1");
	CurrentMapIndex = 0;

	MapNames.Add(TEXT("L_Title"));
	MapNames.Add(TEXT("L_Lobby"));
	MapNames.Add(TEXT("L_Round1"));
	MapNames.Add(TEXT("L_Round2"));
	MapNames.Add(TEXT("L_Result"));
}

FString UPRGameInstance::GetCurrentMapName() const
{
	if (MapNames.IsValidIndex(CurrentMapIndex))
	{
		return MapNames[CurrentMapIndex];
	}

	return FString();
}

bool UPRGameInstance::TravelToMapByIndex(int32 NewMapIndex)
{
	if (!MapNames.IsValidIndex(NewMapIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] Invalid Map Index: %d"), NewMapIndex);
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] World is null"));
		return false;
	}

	CurrentMapIndex = NewMapIndex;
	const FString TargetMapName = MapNames[CurrentMapIndex];

	UE_LOG(LogTemp, Warning, TEXT("[GameInstance] TravelToMapByIndex -> %s"), *TargetMapName);

	World->ServerTravel(TargetMapName + TEXT("?Listen"));
	return true;
}

bool UPRGameInstance::TravelToNextMap()
{
	const int32 NextIndex = CurrentMapIndex + 1;
	return TravelToMapByIndex(NextIndex);
}

bool UPRGameInstance::TravelToResultMap()
{
	const int32 ResultIndex = MapNames.IndexOfByKey(TEXT("L_Result"));
	if (ResultIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] L_Result not found in MapNames"));
		return false;
	}

	return TravelToMapByIndex(ResultIndex);
}

void UPRGameInstance::ResetMapFlow()
{
	const int32 Round1Index = MapNames.IndexOfByKey(TEXT("L_Round1"));
	if (Round1Index != INDEX_NONE)
	{
		CurrentMapIndex = Round1Index;
	}
	else
	{
		CurrentMapIndex = 0;
	}
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
}

void UPRGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		const int32 Round1Index = MapNames.IndexOfByKey(TEXT("L_Round1"));
		if (Round1Index != INDEX_NONE)
		{
			CurrentMapIndex = Round1Index;
			GameMapName = MapNames[CurrentMapIndex];
		}

		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(GameMapName + TEXT("?Listen"));
		}
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

void UPRGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}