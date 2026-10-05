#pragma once

#include "CoreMinimal.h"
#include "MenuPlayerController.h"
#include "LobbyPlayerController.generated.h"



DECLARE_DELEGATE(FOnSwitchToHeroSelection)

UCLASS()
class ALobbyPlayerController : public AMenuPlayerController
{
	GENERATED_BODY()
	ALobbyPlayerController();
public:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestSlotSelectionChange(uint8 NewSlot);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartHeroSelection();

	UFUNCTION(Client, Reliable)
	void Client_StartHeroSelection();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartMatch();

	FOnSwitchToHeroSelection OnSwitchToHeroSelection;
};
