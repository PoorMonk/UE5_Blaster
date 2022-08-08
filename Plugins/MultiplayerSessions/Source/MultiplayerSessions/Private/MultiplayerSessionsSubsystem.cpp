// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	m_createSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	m_findSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	m_joinSessionsCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	m_startSessionCompleteDelagate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	m_destroySessionCompleteDelagete(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem)
	{
		m_onlineSessionInterface = onlineSubsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 numPublicConnections, FString matchType)
{
	if (!m_onlineSessionInterface.IsValid())
	{
		return;
	}

	// 如果已经存在会话，先销毁再创建
	auto existingSession = m_onlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (existingSession)
	{
		m_bCreateSessionOnDestroy = true;
		m_lastNumPublicConnections = numPublicConnections;
		m_lastMatchType = matchType;
		DestroySession();
		return;
	}

	// 添加创建会话委托
	m_createSessionCompleteDelegateHandle = m_onlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(m_createSessionCompleteDelegate);

	// 设置会话参数
	m_lastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	m_lastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	m_lastSessionSettings->NumPublicConnections = numPublicConnections;
	m_lastSessionSettings->bAllowJoinInProgress = true;
	m_lastSessionSettings->bAllowJoinViaPresence = true;
	m_lastSessionSettings->bShouldAdvertise = true;
	m_lastSessionSettings->bUsesPresence = true;
	m_lastSessionSettings->bUseLobbiesIfAvailable = true;
	m_lastSessionSettings->Set(FName("MatchType"), matchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	m_lastSessionSettings->BuildUniqueId = 1;
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (localPlayer)
	{
		if (!m_onlineSessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *m_lastSessionSettings))
		{
			m_onlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(m_createSessionCompleteDelegateHandle);
			
			multiplayerOnCreateSessionComplete.Broadcast(false);		
		}
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 maxSearchResults)
{
	if (!m_onlineSessionInterface.IsValid())
	{
		return;
	}

	m_findSessionsCompleteDelegateHandle = m_onlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(m_findSessionsCompleteDelegate);

	m_lastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	m_lastSessionSearch->MaxSearchResults = maxSearchResults;
	m_lastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	m_lastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (localPlayer)
	{
		if (!m_onlineSessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), m_lastSessionSearch.ToSharedRef()))
		{
			m_onlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(m_findSessionsCompleteDelegateHandle);
			multiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		}
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& sessionResult)
{
	if (!m_onlineSessionInterface.IsValid())
	{
		multiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	m_joinSessionsCompleteDelegateHandle = m_onlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(m_joinSessionsCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (localPlayer)
	{
		if (!m_onlineSessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, sessionResult))
		{
			m_onlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(m_joinSessionsCompleteDelegateHandle);
			multiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		}
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!m_onlineSessionInterface.IsValid())
	{
		return;
	}

	m_onlineSessionInterface->StartSession(NAME_GameSession);
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!m_onlineSessionInterface.IsValid())
	{
		multiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	m_destroySessionCompleteDelageteHandle = m_onlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(m_destroySessionCompleteDelagete);

	if (!m_onlineSessionInterface->DestroySession(NAME_GameSession))
	{
		m_onlineSessionInterface->ClearOnEndSessionCompleteDelegate_Handle(m_destroySessionCompleteDelageteHandle);
		multiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (m_onlineSessionInterface)
	{
		m_onlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(m_createSessionCompleteDelegateHandle);
	}

	multiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (m_onlineSessionInterface)
	{
		m_onlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(m_findSessionsCompleteDelegateHandle);
	}

	if (m_lastSessionSearch->SearchResults.Num() <= 0)
	{
		multiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	multiplayerOnFindSessionsComplete.Broadcast(m_lastSessionSearch->SearchResults, true);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type type)
{
	if (m_onlineSessionInterface)
	{
		m_onlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(m_joinSessionsCompleteDelegateHandle);
	}

	multiplayerOnJoinSessionComplete.Broadcast(type);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (m_onlineSessionInterface)
	{
		m_onlineSessionInterface->ClearOnStartSessionCompleteDelegate_Handle(m_startSessionCompleteDelagateHandle);
	}

	multiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (m_onlineSessionInterface)
	{
		m_onlineSessionInterface->ClearOnEndSessionCompleteDelegate_Handle(m_destroySessionCompleteDelageteHandle);
	}

	if (bWasSuccessful && m_bCreateSessionOnDestroy)
	{
		m_bCreateSessionOnDestroy = false;
		CreateSession(m_lastNumPublicConnections, m_lastMatchType);
	}

	multiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}
