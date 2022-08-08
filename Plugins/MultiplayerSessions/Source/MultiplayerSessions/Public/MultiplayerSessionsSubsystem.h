// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
// 参数类型不是UENUM,UCLASS，所以不使用DYNAMIC的委托，绑定的函数就可以不用加UFUNCTION
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UMultiplayerSessionsSubsystem();

	//
	// To handle session functionality. The Menu class will call these
	//
	UFUNCTION(BlueprintCallable)
	void CreateSession(int32 numPublicConnections, FString matchType);

	UFUNCTION(BlueprintCallable)
	void FindSessions(int32 maxSearchResults);

	void JoinSession(const FOnlineSessionSearchResult& sessionResult);

	void StartSession();

	void DestroySession();

protected:

	//
	// Internal callbacks for the delegates we'll add to the Online Session Interface delegate list.
	// Thise don't need to be called outside this class.
	//
	void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type type);
	void OnStartSessionComplete(FName sessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName sessionName, bool bWasSuccessful);


public:

	FMultiplayerOnCreateSessionComplete multiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete multiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete multiplayerOnJoinSessionComplete;
	FMultiplayerOnStartSessionComplete multiplayerOnStartSessionComplete;
	FMultiplayerOnDestroySessionComplete multiplayerOnDestroySessionComplete;

private:
	// Pointer to online session interface
	IOnlineSessionPtr m_onlineSessionInterface;
	TSharedPtr<FOnlineSessionSettings> m_lastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> m_lastSessionSearch;

	//
	// To add to the Online Session Interface delegate list.
	// We'll bind our MultiplayerSessionsSubsystem internal callbacks to these.
	//

	FOnCreateSessionCompleteDelegate m_createSessionCompleteDelegate;
	FDelegateHandle m_createSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate m_findSessionsCompleteDelegate;
	FDelegateHandle m_findSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate m_joinSessionsCompleteDelegate;
	FDelegateHandle m_joinSessionsCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate m_startSessionCompleteDelagate;
	FDelegateHandle m_startSessionCompleteDelagateHandle;

	FOnDestroySessionCompleteDelegate m_destroySessionCompleteDelagete;
	FDelegateHandle m_destroySessionCompleteDelageteHandle;

	bool m_bCreateSessionOnDestroy{ false };
	int32 m_lastNumPublicConnections;
	FString m_lastMatchType;
};
