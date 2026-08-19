#include "CAbilitySystemComponent.h"

#include "Conditions/MovieSceneScalabilityCondition.h"

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	/*Explain ：为什么写在SystemComponent里也能影响到Attribute？
	Character / PlayerState
	│
	├── AbilitySystemComponent
	│
	└── AttributeSet
	*/
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	for (const TSubclassOf<UGameplayEffect>& EffectClass : InitialEffect)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass , 1 , MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::GiveInitialAbility()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	for (const TSubclassOf<UGameplayAbility>& Ability : GameplayAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(Ability,0,-1,nullptr));
	}
	
	for (const TSubclassOf<UGameplayAbility>& Ability : BasicGameplayAbilities)
	{
		//Level > 0  means you learn the Ability;
		GiveAbility(FGameplayAbilitySpec(Ability,1,-1,nullptr));
	}
}
