#include "CAbilitySystemStatics.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicAttack");
}

FGameplayTag UCAbilitySystemStatics::GetDeathStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.death");
}

FGameplayTag UCAbilitySystemStatics::GetStunStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.stun");
}

FGameplayTag UCAbilitySystemStatics::GetBasicAttackPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicAttack.pressed");
}
