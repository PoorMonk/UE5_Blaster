// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString InText)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(InText));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	if (InPawn == nullptr) return;
	ENetRole remoteRole = InPawn->GetRemoteRole();
	APlayerState* playerState = InPawn->GetPlayerState();
	FString playerName = FString("");
	if (playerState)
	{
		playerName = playerState->GetPlayerName();
	}
	FString strRemoteRole;
	switch (remoteRole)
	{
	case ENetRole::ROLE_Authority:
		strRemoteRole = FString("ROLE_Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		strRemoteRole = FString("ROLE_AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		strRemoteRole = FString("ROLE_SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		strRemoteRole = FString("None");
		break;
	default:
		break;
	}

	//FString strShowText = FString::Printf(TEXT("Remote Role : %s"), *strRemoteRole);
	FString strShowText = FString::Printf(TEXT("Player name is : %s"), *playerName);
	SetDisplayText(strShowText);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	if (InPawn == nullptr) return;
	APlayerState* playerState = InPawn->GetPlayerState();
	FString playerName = FString("");
	if (playerState)
	{
		playerName = playerState->GetPlayerName();
	}
	FString strShowText = FString::Printf(TEXT("Player name is : %s"), *playerName);
	SetDisplayText(strShowText);
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
