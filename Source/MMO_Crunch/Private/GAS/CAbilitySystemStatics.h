#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAbilitySystemStatics.generated.h"

UCLASS()
class UCAbilitySystemStatics  :public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FGameplayTag GetBasicAttackAbilityTag();
	static FGameplayTag GetDeathStatsAbilityTag();
	static FGameplayTag GetStunStatsAbilityTag();
	static FGameplayTag GetBasicAttackPressedTag();

	static float GetStaticCooldownDurationFromAbility(const class UGameplayAbility* Ability);
	static float GetStaticCostFromAbility(const UGameplayAbility* Ability);
};