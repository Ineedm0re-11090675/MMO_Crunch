#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "GenericTeamAgentInterface.h"
#include "CPlayerState.generated.h"

/**
 * 
 */
class UPA_CharacterDefinition;

UCLASS()
class ACPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ACPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;	

	virtual void BeginPlay() override;

	virtual void CopyProperties(APlayerState* PlayerState) override;

	TSubclassOf<APawn> GetSelectedPawnClass() const;

	FGenericTeamId GetTeamIDBasedOnSlot() const;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSelectedCharacterDefinition(const UPA_CharacterDefinition* SelectedDefinition);
private:
	UPROPERTY(Replicated)
	FPlayerSelection PlayerSelection;

	UPROPERTY()
	class ACGameState* CGameState;

	void PlayerSelectionUpdated(const TArray<struct FPlayerSelection>& NewPlayerSelections);
};