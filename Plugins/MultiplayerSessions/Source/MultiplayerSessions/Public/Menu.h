// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessionsSubsystem.h"

#include "Menu.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type type);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

private:
	UFUNCTION()
	void HostBtnClicked();

	UFUNCTION()
	void JoinBtnClicked();

	void MenuTearDown();

private:

	UPROPERTY(meta = (BindWidget))
	UButton* HostBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinBtn;

	class UMultiplayerSessionsSubsystem* m_multiplayerSessionSubSystem;

	int32 m_numPublicConnections{ 4 };
	FString m_typeOfMatch{ "FreeForAll" };
	FString m_lobbyPath{ FString(TEXT("")) };
};
