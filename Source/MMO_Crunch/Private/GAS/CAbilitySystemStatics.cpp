#include "CAbilitySystemStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicAttack");
}

FGameplayTag UCAbilitySystemStatics::GetDeathStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.death");
}

FGameplayTag UCAbilitySystemStatics::GetAimingStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.aiming");
}

FGameplayTag UCAbilitySystemStatics::GetStunStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.stun");
}

FGameplayTag UCAbilitySystemStatics::GetBasicAttackPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicAttack.pressed");
}

FGameplayTag UCAbilitySystemStatics::GetCameraShakeGameplayCueTag()
{
	return FGameplayTag::RequestGameplayTag("GameplayCue.hit.reaction"); 
}

FGameplayTag UCAbilitySystemStatics::GetFullHealthStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.health.full");  
}

FGameplayTag UCAbilitySystemStatics::GetFullManaStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.mana.full");  
}

FGameplayTag UCAbilitySystemStatics::GetEmptyHealthStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.health.empty");  
}

FGameplayTag UCAbilitySystemStatics::GetEmptyManaStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.mana.empty");   
}

bool UCAbilitySystemStatics::IsHero(const AActor* ActorToCheck)
{
	const IAbilitySystemInterface* ActorISA = Cast<IAbilitySystemInterface>(ActorToCheck);
	UAbilitySystemComponent* ActorASC = ActorISA ? ActorISA->GetAbilitySystemComponent() : nullptr;
	if (ActorASC)
	{
		return  ActorASC->HasMatchingGameplayTag(GetHeroTag());
	}
	return false;
}

FGameplayTag UCAbilitySystemStatics::GetHeroTag()
{
	return FGameplayTag::RequestGameplayTag("role.hero");   
}

FGameplayTag UCAbilitySystemStatics::GetExpAttributeTag()
{
	return FGameplayTag::RequestGameplayTag("attr.experience");   
}

FGameplayTag UCAbilitySystemStatics::GetGoldAttributeTag()
{
	return FGameplayTag::RequestGameplayTag("attr.gold"); 
}

float UCAbilitySystemStatics::GetStaticCooldownDurationFromAbility(const UGameplayAbility* Ability)
{
	if (!Ability) return 0.f;

	const UGameplayEffect* CooldownEffect =Ability->GetCooldownGameplayEffect();
	if (!CooldownEffect) return 0.f;

	float CooldownDuration = 0.f;
	CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1,CooldownDuration);
	return CooldownDuration;
}

float UCAbilitySystemStatics::GetStaticCostFromAbility(const UGameplayAbility* Ability)
{
	if (!Ability) return 0.f;

	const UGameplayEffect* CostEffect =Ability->GetCostGameplayEffect();
	if (!CostEffect  || CostEffect ->Modifiers.Num() == 0 ) return 0.f;

	float Cost = 0.f;
	CostEffect->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(1,Cost);
	return FMath::Abs(Cost) ; 
}
