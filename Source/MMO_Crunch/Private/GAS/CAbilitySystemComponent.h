#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CAbilitySystemComponent.generated.h"

UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	void ApplyInitialEffects();
	void GiveInitialAbility();
private:
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffect;

	/*
	 *特殊技能，习得技能
	 */
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayAbility>> GameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayAbility>> BasicGameplayAbilities; 
};
