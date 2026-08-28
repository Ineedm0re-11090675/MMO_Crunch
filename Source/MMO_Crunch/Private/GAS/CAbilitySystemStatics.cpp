#include "CAbilitySystemStatics.h"
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
