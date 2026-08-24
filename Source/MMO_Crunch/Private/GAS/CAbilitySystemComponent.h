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
	void ApplyInitialEffects();
	void GiveInitialAbility();
	void ApplyFullStatsEffect();
private:
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect,int Level = 1);
	
	void UpdateHealth(const FOnAttributeChangeData& ChangedData);
	
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffect;
	
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatsEffect;
	/*
	 *特殊技能，习得技能
	 */
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>> GameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>> BasicGameplayAbilities; 
};
