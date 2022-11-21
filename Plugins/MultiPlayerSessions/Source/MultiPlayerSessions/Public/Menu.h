// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"


UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 numOfPublicConnections = 4, 
				   FString typeOfMatch = FString(TEXT("FreeForAll")), 
				   FString lobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")) );
	// C:/UnrealProjects/MultiPlayer/MenuSystem/Content/ThirdPerson/Maps/Lobby.umap


protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* inLevel, UWorld* inWorld) override;

/// Callback for the custom delegates on the multiplayer session subsystem
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	///  The subsystem designed to handle all online session functionality
	class UMultiplayerSessionSubsystem* MultiplayerSessionsSubsystem;

	int32 NumberPublicConnections {4};
	FString MatchType {TEXT("FreeForAll")};
	FString PathToLobby{ TEXT("") };
};
