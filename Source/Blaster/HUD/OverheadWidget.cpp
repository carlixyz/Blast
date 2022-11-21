// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString textToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(textToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* inPawn)
{
	ENetRole remoteRole = inPawn->GetRemoteRole();
	FString Role = "Undef";
	switch (remoteRole)
	{
	case ROLE_None:
		Role = FString("None");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	}

	FString remoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(remoteRoleString);
}

void UOverheadWidget::SetDisplayName(FString textToDisplay)
{
	if (DisplayName)
	{
		DisplayName->SetText(FText::FromString(textToDisplay));
	}
}

void UOverheadWidget::ShowPlayerName(APawn* inPawn)
{
	if (inPawn)
	{
		APlayerState* playerState = inPawn->GetPlayerState();

		if (playerState)
		{
			FString pName = playerState->GetPlayerName();
			SetDisplayName(pName);
		}
	}
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* inLevel, UWorld* inWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(inLevel, inWorld);
}
