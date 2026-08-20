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
	void ApplyInitialEffects();
	void GiveInitialAbility();
private:
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffect;

	/*
	 *特殊技能，习得技能
	 */
	// Codex: Lessons 34-35 - Store the input ID beside each ability class so
	// GiveAbility can create specs that respond to Enhanced Input.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>> GameplayAbilities;

	// Codex: Lessons 34-35 - Basic abilities use the same input-ID path as
	// learned abilities, while retaining their existing level distinction.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>> BasicGameplayAbilities;
};
