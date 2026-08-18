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
