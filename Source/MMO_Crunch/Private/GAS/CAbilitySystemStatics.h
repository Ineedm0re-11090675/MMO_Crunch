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
	static FGameplayTag GetAimingStatsAbilityTag();
	static FGameplayTag GetStunStatsAbilityTag();
	static FGameplayTag GetBasicAttackPressedTag();
	static FGameplayTag GetCameraShakeGameplayCueTag();
	static FGameplayTag GetFullHealthStatsAbilityTag();
	static FGameplayTag GetFullManaStatsAbilityTag();
	static FGameplayTag GetEmptyHealthStatsAbilityTag();
	static FGameplayTag GetEmptyManaStatsAbilityTag();
	static bool IsHero(const AActor* ActorToCheck);
	static FGameplayTag GetHeroTag();
	static FGameplayTag GetExpAttributeTag();
	static FGameplayTag GetGoldAttributeTag();
	
	static float GetStaticCooldownDurationFromAbility(const class UGameplayAbility* Ability);
	static float GetStaticCostFromAbility(const UGameplayAbility* Ability);
};