// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* world = GetWorld();
	if (world)
	{
		APlayerController* playerController = world->GetFirstPlayerController();
		if (playerController)
		{
			FInputModeUIOnly inputMode;
			inputMode.SetWidgetToFocus(TakeWidget());
			inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			playerController->SetInputMode(inputMode);
			playerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* gameInstance = GetGameInstance();
	if (gameInstance)
	{
		m_multiplayerSessionSubSystem = gameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (m_multiplayerSessionSubSystem)
	{
		m_multiplayerSessionSubSystem->multiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		m_multiplayerSessionSubSystem->multiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		m_multiplayerSessionSubSystem->multiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		m_multiplayerSessionSubSystem->multiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		m_multiplayerSessionSubSystem->multiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
	}

	m_numPublicConnections = NumberOfPublicConnections;
	m_typeOfMatch = TypeOfMatch;
	m_lobbyPath = FString::Printf(TEXT("%s?listen"), *LobbyPath);
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostBtn)
	{
		HostBtn->OnClicked.AddDynamic(this, &ThisClass::HostBtnClicked);
	}

	if (JoinBtn)
	{
		JoinBtn->OnClicked.AddDynamic(this, &ThisClass::JoinBtnClicked);
	}

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UWorld* world = GetWorld();
		if (world)
		{
			world->ServerTravel(m_lobbyPath);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("Failed to create session")));
			HostBtn->SetIsEnabled(true);
		}
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful)
{
	if (!m_multiplayerSessionSubSystem)
	{
		return;
	}

	for (auto result : sessionResults)
	{
		FString matchType;
		result.Session.SessionSettings.Get(FName("MatchType"), matchType);
		if (matchType == m_typeOfMatch)
		{
			m_multiplayerSessionSubSystem->JoinSession(result);
			return;
		}
	}

	if (!bWasSuccessful || sessionResults.Num() <= 0)
	{
		JoinBtn->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type type)
{
	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		IOnlineSessionPtr sessionInterface = subsystem->GetSessionInterface();
		if (sessionInterface.IsValid())
		{
			FString address;
			sessionInterface->GetResolvedConnectString(NAME_GameSession, address);

			APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (playerController)
			{
				playerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
			}
		}
	}

	if (type == EOnJoinSessionCompleteResult::Success)
	{
		JoinBtn->SetIsEnabled(true);
	}
}

void UMenu::OnStartSession(bool bWasSuccessful)
{

}

void UMenu::OnDestroySession(bool bWasSuccessful)
{

}

void UMenu::HostBtnClicked()
{
	HostBtn->SetIsEnabled(false);
	if (m_multiplayerSessionSubSystem)
	{
		m_multiplayerSessionSubSystem->CreateSession(m_numPublicConnections, m_typeOfMatch);
	}
}

void UMenu::JoinBtnClicked()
{
	JoinBtn->SetIsEnabled(false);
	if (m_multiplayerSessionSubSystem)
	{
		m_multiplayerSessionSubSystem->FindSessions(10000);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* world = GetWorld();
	if (world)
	{
		APlayerController* playerController = world->GetFirstPlayerController();
		if (playerController)
		{
			FInputModeGameOnly inputMode;
			playerController->SetInputMode(inputMode);
			playerController->SetShowMouseCursor(false);
		}
	}
}
