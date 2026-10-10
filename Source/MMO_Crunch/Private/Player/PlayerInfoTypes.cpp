#include "PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "Network/CNetStatic.h"
FPlayerSelection::FPlayerSelection()
	:Slot{GetInvalidSlot()} , PlayerUniqueId{FUniqueNetIdRepl::Invalid()}, PlayerNickName{},CharacterDefinition{nullptr}
{
}

FPlayerSelection::FPlayerSelection(uint8 InSlot, const APlayerState* InPlayerState)
	:Slot(InSlot),CharacterDefinition{nullptr}
{
	if (InPlayerState)
	{
		PlayerUniqueId = InPlayerState->GetUniqueId();
		PlayerNickName = InPlayerState->GetPlayerName();
	}
}

bool FPlayerSelection::IsForPlayer(const APlayerState* InPlayerState) const
{
	if (!InPlayerState) return false;
	// Codex: Server builds also need this comparison and a return value.
	return PlayerUniqueId.IsValid()
		   && InPlayerState->GetUniqueId() == PlayerUniqueId;
}

bool FPlayerSelection::IsValid() const
{
#if WITH_EDITOR
	return true;
#else
	if (!PlayerUniqueId.IsValid())
	{
		return false;
	}
	if (Slot == GetInvalidSlot())
	{
		return false;
	}
	if (Slot >= UCNetStatic::GetPlayerCountPerTeam()*2)
	{
		return false;
	}
	// Codex: Fix the return keyword so non-editor targets compile.
	return true;
#endif
}

uint8 FPlayerSelection::GetInvalidSlot()
{
	return uint8(255);
}
