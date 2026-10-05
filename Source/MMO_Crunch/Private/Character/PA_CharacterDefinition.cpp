#include "PA_CharacterDefinition.h"

#include "CCharacter.h"

FPrimaryAssetId UPA_CharacterDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetPrimaryAssetType(),GetFName());
}

FPrimaryAssetType UPA_CharacterDefinition::GetPrimaryAssetType()
{
	return FPrimaryAssetType("CharacterDefination");
}

UTexture2D* UPA_CharacterDefinition::LoadIcon() const
{
	CharacterIcon.LoadSynchronous(); 
	if (CharacterIcon.IsValid())
	{
		return CharacterIcon.Get();
	}
	return nullptr;
}

TSubclassOf<ACCharacter> UPA_CharacterDefinition::LoadCharacterClass() const
{
	CharacterClass.LoadSynchronous();
	if (CharacterClass.IsValid())
	{
		return CharacterClass.Get();
	}
	return TSubclassOf<ACCharacter>();
}

TSubclassOf<UAnimInstance> UPA_CharacterDefinition::LoadDisplayAnimBP() const
{
	DisplayAnimBP.LoadSynchronous();
	if (DisplayAnimBP.IsValid())
	{
		return DisplayAnimBP.Get();
	}
	return TSubclassOf<UAnimInstance>();
}

USkeletalMesh* UPA_CharacterDefinition::LoadDisplayMesh() const
{
	TSubclassOf<ACCharacter> LoadedCharacterClass = LoadCharacterClass();

	if (!LoadedCharacterClass) return nullptr;

	const ACCharacter* LoadCharacter = Cast<ACCharacter>(LoadedCharacterClass->GetDefaultObject());
	if (!LoadCharacter) return nullptr;

	return LoadCharacter->GetMesh()->GetSkeletalMeshAsset();
}

const TMap<ECAbilityInputId, TSubclassOf<UGameplayAbility>>* UPA_CharacterDefinition::GetAbilities() const
{
	TSubclassOf<ACCharacter> LoadedCharacterClass = LoadCharacterClass();

	if (!LoadedCharacterClass) return nullptr;

	const ACCharacter* LoadCharacter = Cast<ACCharacter>(LoadedCharacterClass->GetDefaultObject());
	if (!LoadCharacter) return nullptr;

	return &(LoadCharacter->GetAbility());
}

