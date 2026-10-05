#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "PA_CharacterDefinition.generated.h"

UCLASS()
class UPA_CharacterDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetPrimaryAssetType();
	
	FString GetCharacterName() const {return CharacterName;}

	UTexture2D* LoadIcon() const;
	
	TSubclassOf<class ACCharacter> LoadCharacterClass() const;
	
	TSubclassOf<UAnimInstance> LoadDisplayAnimBP() const;
	
	USkeletalMesh* LoadDisplayMesh() const ;

	const TMap<ECAbilityInputId,TSubclassOf<class UGameplayAbility>>* GetAbilities() const;
private:
	UPROPERTY(EditDefaultsOnly,Category = "Character")
	FString CharacterName;

	UPROPERTY(EditDefaultsOnly,Category = "Character")
	TSoftObjectPtr<UTexture2D> CharacterIcon;
	
	UPROPERTY(EditDefaultsOnly,Category = "Character")
	TSoftClassPtr<ACCharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly,Category = "Character")
	TSoftClassPtr<UAnimInstance> DisplayAnimBP;

	
};
