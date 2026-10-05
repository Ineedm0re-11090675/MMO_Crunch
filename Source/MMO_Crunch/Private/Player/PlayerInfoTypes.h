#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.generated.h"


class APlayerState;
class UPA_CharacterDefinition;

USTRUCT()
struct FPlayerSelection
{
	GENERATED_BODY()
public:
	FPlayerSelection();
	
	FPlayerSelection(uint8 InSlot,const APlayerState* InPlayerState);

	FORCEINLINE void SetSlot(uint8 InSlot) { Slot=InSlot;}
	FORCEINLINE uint8 GetSlot() const { return Slot;}
	FORCEINLINE FUniqueNetIdRepl GetPlayerUniqueId() const { return PlayerUniqueId; }
	FORCEINLINE FString GetPlayerNickName() const { return PlayerNickName; }
	FORCEINLINE const UPA_CharacterDefinition* GetCharacterDefinition() const { return CharacterDefinition; }
	FORCEINLINE void SetCharacterDefinition(const UPA_CharacterDefinition* InCharacterDefinition){ CharacterDefinition =InCharacterDefinition; }
	
	bool IsForPlayer(const APlayerState* InPlayerState) const ;
	bool IsValid() const ;
	
	static uint8 GetInvalidSlot();
private:
	UPROPERTY()
	uint8 Slot;

	UPROPERTY()
	FUniqueNetIdRepl PlayerUniqueId;

	UPROPERTY()
	FString PlayerNickName;

	UPROPERTY()
	const UPA_CharacterDefinition* CharacterDefinition;
};