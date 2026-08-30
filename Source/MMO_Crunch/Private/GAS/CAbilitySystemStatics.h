#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAbilitySystemStatics.generated.h"

class UAbilitySystemComponent;
struct FGameplayAbilitySpec;
class UGameplayAbility;
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
	static bool IsAbilityAtMaxLevel(const FGameplayAbilitySpec& AbilitySpec);
	
	static float GetStaticCooldownDurationFromAbility(const UGameplayAbility* Ability);
	static float GetStaticCostFromAbility(const UGameplayAbility* Ability);

	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& AbilitySystemComponent);

	static float GetManaCostFor(const UGameplayAbility* AbilityCOD,const UAbilitySystemComponent& OwnASC,int Level);
	static float GetCooldownFor(const UGameplayAbility* AbilityCOD,const UAbilitySystemComponent& OwnASC,int Level);
	static float GetCooldownRemainingFor(const UGameplayAbility* AbilityCOD,const UAbilitySystemComponent& OwnASC);
};