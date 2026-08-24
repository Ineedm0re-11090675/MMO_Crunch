#include "CAbilitySystemStatics.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicAttack");
}

FGameplayTag UCAbilitySystemStatics::GetDeathStatsAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("stats.death");
}
