#pragma once

#include "CoreMinimal.h"
#include  "GameFramework/GameStateBase.h"
#include "Player/PlayerInfoTypes.h"
#include "CGameState.generated.h"

class UPA_CharacterDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSelectionUpdated, const TArray<FPlayerSelection>& /*New Player Selection*/)
UCLASS()
class ACGameState : public AGameStateBase
{
	GENERATED_BODY()
public:

	void RequestPlayerSelectionChange(const APlayerState* RequestPlayer,uint8 DesireSlot);

	bool IsDefinitionOccupied(const UPA_CharacterDefinition* InCharacterDefinition) const;
	
	bool IsSlotOccupied(uint8 SlotID) const ;
	
	FOnPlayerSelectionUpdated OnPlayerSelectionUpdated;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	TArray<FPlayerSelection> GetPlayerSelectionArray() const;

	bool CanStartHeroSelection() const;

	bool CanStartMatch() const;

	void SetCharacterSelected(const APlayerState* RequestingPlayer,const UPA_CharacterDefinition* SelectedDefinition);

	void SetCharacterDeselected(const UPA_CharacterDefinition* DeselectedDefinition);
private:
		UPROPERTY(ReplicatedUsing= OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	UFUNCTION()
	void OnRep_PlayerSelectionArray();
};
