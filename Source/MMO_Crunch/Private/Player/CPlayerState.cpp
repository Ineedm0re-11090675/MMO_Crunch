#include "CPlayerState.h"

#include "Character/PA_CharacterDefinition.h"
#include "FrameWork/CGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Character/CCharacter.h"
#include "Network/CNetStatic.h"

ACPlayerState::ACPlayerState()
{
	bReplicates = true;
	SetNetUpdateFrequency(100.f);
}

void ACPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACPlayerState,PlayerSelection); 
}

void ACPlayerState::BeginPlay()
{
	Super::BeginPlay();

	CGameState = Cast<ACGameState>(UGameplayStatics::GetGameState(this));
	if (CGameState)
	{
		CGameState->OnPlayerSelectionUpdated.AddUObject(this,&ACPlayerState::PlayerSelectionUpdated);
	}
}

void ACPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	ACPlayerState* NewPlayerState = Cast<ACPlayerState>(PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->PlayerSelection = PlayerSelection;
	}
}

TSubclassOf<APawn> ACPlayerState::GetSelectedPawnClass() const
{
	if (PlayerSelection.GetCharacterDefinition())
	{
		return PlayerSelection.GetCharacterDefinition()->LoadCharacterClass();
	}
	return nullptr;
}

FGenericTeamId ACPlayerState::GetTeamIDBasedOnSlot() const
{
	//Codex: An unassigned/out-of-range slot is not team 1 during seamless travel
	
	if (PlayerSelection.GetSlot() >= UCNetStatic::GetPlayerCountPerTeam() * 2)
	{
		return FGenericTeamId::NoTeam;
	}
	return PlayerSelection.GetSlot() < UCNetStatic::GetPlayerCountPerTeam() ? FGenericTeamId{0}:FGenericTeamId{1};
}

void ACPlayerState::Server_SetSelectedCharacterDefinition_Implementation(
	const UPA_CharacterDefinition* SelectedDefinition)
{
	if (!SelectedDefinition) return;

	if (!CGameState) return;

	if (CGameState->IsDefinitionOccupied(SelectedDefinition)) return;

	if (PlayerSelection.GetCharacterDefinition())
	{
		CGameState->SetCharacterDeselected(PlayerSelection.GetCharacterDefinition());
	}
	//这之前已经保证了每个PlayerState都有对应的PlayerSelection
	//这里是Server的赋值修改
	PlayerSelection.SetCharacterDefinition(SelectedDefinition);
	//这里是到CGameState里BroadCast给Client
	CGameState->SetCharacterSelected(this, SelectedDefinition);
}

bool ACPlayerState::Server_SetSelectedCharacterDefinition_Validate(const UPA_CharacterDefinition* SelectedDefinition)
{
	return true;
}

void ACPlayerState::PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections)
{
	for (const FPlayerSelection& NewPlayerSelection : NewPlayerSelections)
	{
		if (NewPlayerSelection.IsForPlayer(this))
		{
			PlayerSelection = NewPlayerSelection;
			return;
		}
	}
}
