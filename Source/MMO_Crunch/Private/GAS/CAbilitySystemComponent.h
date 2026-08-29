#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CGameplayAbilityTypes.h"
#include "CAbilitySystemComponent.generated.h"

UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UCAbilitySystemComponent();
	void ServerSideInit();
	void InitializeBaseAttributes();
	void ApplyFullStatsEffect();

	bool bIsMaxLevel() const ;
	
	const TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>>& GetAbility() const ; 
private:

	void ApplyInitialEffects();
	void GiveInitialAbility();
	
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect,int Level = 1);
	
	void UpdateHealth(const FOnAttributeChangeData& ChangedData);
	void UpdateMana(const FOnAttributeChangeData& ChangedData);
	void UpdateExperience(const FOnAttributeChangeData& ChangedData);
	

	/*
	 *特殊技能，习得技能
	 */
	
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Ability")
	TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>> GameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Ability")
	TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>> BasicGameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Ability")
	class UPA_AbilitySystemGenerics* AbilitySystemGenerics ;
};
