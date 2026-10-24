#include "LobbyPlayerController.h"

#include "FrameWork/CGameInstance.h"
#include "FrameWork/CGameState.h"
#include "GameFramework/PlayerState.h"
void ALobbyPlayerController::Server_RequestSlotSelectionChange_Implementation(uint8 NewSlot)
{
	if (!GetWorld()) return;

	ACGameState* CGameState = GetWorld()->GetGameState<ACGameState>();
	if (!CGameState) return;

	CGameState->RequestPlayerSelectionChange(GetPlayerState<APlayerState>(), NewSlot);
	
}

bool ALobbyPlayerController::Server_RequestSlotSelectionChange_Validate(uint8 NewSlot)
{
	return true;
}

void ALobbyPlayerController::Server_StartHeroSelection_Implementation()
{
	if (!GetWorld() || !HasAuthority()) return;

	for (FConstPlayerControllerIterator PlayerControllerIterator = GetWorld()->GetPlayerControllerIterator() ; PlayerControllerIterator ; ++PlayerControllerIterator)
	{
		ALobbyPlayerController* PlayerController = Cast<ALobbyPlayerController>(*PlayerControllerIterator);
		if (PlayerController)
		{
			PlayerController->Client_StartHeroSelection();
		}
	}
}

bool ALobbyPlayerController::Server_StartHeroSelection_Validate()
{
	return true;
}

void ALobbyPlayerController::Client_StartHeroSelection_Implementation()
{
	OnSwitchToHeroSelection.ExecuteIfBound();
}

ALobbyPlayerController::ALobbyPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void ALobbyPlayerController::Server_RequestStartMatch_Implementation()
{
	ACGameState* GameState = GetWorld()->GetGameState<ACGameState>();
	if (!GameState || !GameState->CanStartMatch())
	{
		return;
	}
	
	UCGameInstance* CGameInstance = GetGameInstance<UCGameInstance>();
	if (CGameInstance)
	{
		CGameInstance->StartMatch();
	}
}

bool ALobbyPlayerController::Server_RequestStartMatch_Validate()
{
	return true;
}
