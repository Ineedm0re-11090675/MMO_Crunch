#include "CGameState.h"
#include "Net/UnrealNetwork.h"

void ACGameState::OnRep_PlayerSelectionArray()
{
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

void ACGameState::RequestPlayerSelectionChange(const APlayerState* RequestPlayer, uint8 DesireSlot)
{
	if (!HasAuthority() || IsSlotOccupied(DesireSlot)) return;

	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.IsForPlayer (RequestPlayer);
	}
	);
	if (PlayerSelectionPtr)
	{
		PlayerSelectionPtr->SetSlot(DesireSlot);
	}
	else
	{
		PlayerSelectionArray.Add(FPlayerSelection(DesireSlot,RequestPlayer));
	}
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	
}

bool ACGameState::IsDefinitionOccupied(const UPA_CharacterDefinition* InCharacterDefinition) const
{
	const FPlayerSelection* PlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.GetCharacterDefinition() == InCharacterDefinition;
	});
	return PlayerSelection != nullptr;
}

bool ACGameState::IsSlotOccupied(uint8 SlotID) const
{
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if (SlotID == PlayerSelection.GetSlot())
		{
			return true;
		}
	}
	return false;
}

TArray<FPlayerSelection> ACGameState::GetPlayerSelectionArray() const
{
	return PlayerSelectionArray;
}

bool ACGameState::CanStartHeroSelection() const
{
	return PlayerSelectionArray.Num() == PlayerArray.Num();
}

bool ACGameState::CanStartMatch() const
{
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if (PlayerSelection.GetCharacterDefinition() == nullptr)
		{
			return false;
		}
	}
	return true;
}

void ACGameState::SetCharacterSelected(const APlayerState* RequestingPlayer,
                                       const UPA_CharacterDefinition* SelectedDefinition)
{
	if (IsDefinitionOccupied(SelectedDefinition)) return;

	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.IsForPlayer(RequestingPlayer);
		//去找属于这个玩家的Selection然后赋值,所以要在PlayerState上先绑定
	});
	if (FoundPlayerSelection)
	{
		FoundPlayerSelection->SetCharacterDefinition(SelectedDefinition);
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
}

void ACGameState::SetCharacterDeselected(const UPA_CharacterDefinition* DeselectedDefinition)
{
	if (!DeselectedDefinition) return;
	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.GetCharacterDefinition() == DeselectedDefinition; 
	});
	if (FoundPlayerSelection)
	{
		FoundPlayerSelection->SetCharacterDefinition(nullptr);
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
}

void ACGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME_CONDITION_NOTIFY(ACGameState,PlayerSelectionArray, COND_None, REPNOTIFY_Always); 
}


