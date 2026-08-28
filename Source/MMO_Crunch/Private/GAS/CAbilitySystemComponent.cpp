#include "CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "Conditions/MovieSceneScalabilityCondition.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this, &UCAbilitySystemComponent::UpdateHealth);
	GenericCancelInputID = (int32)ECAbilityInputId::Cancel;
	GenericConfirmInputID = (int32)ECAbilityInputId::Confirm;
	
}

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
	
	for (const TPair<ECAbilityInputId,TSubclassOf<UGameplayAbility>>& AbilityPair : GameplayAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,0,(int32)AbilityPair.Key,nullptr));
	}
	
	for (const TPair<ECAbilityInputId,TSubclassOf<UGameplayAbility>>& AbilityPair : BasicGameplayAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,1,(int32)AbilityPair.Key,nullptr));
	}
}

void UCAbilitySystemComponent::ApplyFullStatsEffect()
{
	AuthApplyGameplayEffect(FullStatsEffect);
}

const TMap<ECAbilityInputId, TSubclassOf<UGameplayAbility>>& UCAbilitySystemComponent::GetAbility() const
{
	return GameplayAbilities;
}

void UCAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect ,Level , MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::UpdateHealth(const FOnAttributeChangeData& ChangedData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (ChangedData.NewValue <= 0 && DeathEffect)
	{
		AuthApplyGameplayEffect(DeathEffect);
	}
}
