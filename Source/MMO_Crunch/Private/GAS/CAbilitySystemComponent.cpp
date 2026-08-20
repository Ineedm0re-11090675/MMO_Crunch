#include "CAbilitySystemComponent.h"

// Codex: Removed the unrelated MovieScene include; this implementation only
// depends on the ability system component declaration above.
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
	
	// Codex: Lessons 34-35 - Pass the enum value into the ability spec as its
	// InputID, allowing AbilityLocalInputPressed/Released to activate it.
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : GameplayAbilities)
	{
		if (!AbilityPair.Value)
		{
			continue;
		}

		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, static_cast<int32>(AbilityPair.Key), nullptr));
	}
	
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicGameplayAbilities)
	{
		if (!AbilityPair.Value)
		{
			continue;
		}

		// Level > 0 means the ability is learned.
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, static_cast<int32>(AbilityPair.Key), nullptr));
	}
}
