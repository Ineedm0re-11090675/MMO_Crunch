#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbilityTypes.generated.h"

UENUM(blueprintType)
enum class ECAbilityInputId :uint8
{
	None  UMETA(DisplayName = "None"),
	BasicAttack UMETA(DisplayName = "BasicAttack"),
	Aim UMETA(DisplayName = "Aim"),
	AbilityOne UMETA(DisplayName = "AbilityOne"),
	AbilityTwo UMETA(DisplayName = "AbilityTwo"),
	AbilityThree UMETA(DisplayName = "AbilityThree"),
	AbilityFour UMETA(DisplayName = "AbilityFour"),
	AbilityFive UMETA(DisplayName = "AbilityFive"),
	Confirm UMETA(DisplayName = "Confirm"),
	Cancel UMETA(DisplayName = "Cancel")
};
USTRUCT(BlueprintType)
struct FGenericDamageEffectDef
{
	GENERATED_BODY()
public:
	FGenericDamageEffectDef();
	// Codex: Keep the serialized legacy GE reference during migration; migrated Crunch damage uses UCombatEffect_Damage.
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	FVector PushVelocity;
	// Codex: Store positive MapleCombat damage separately from legacy GE negative Health modifiers.
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float DamageAmount = 1.0f;
};

USTRUCT(BlueprintType)
struct FHeroBaseStats :public FTableRowBase
{

	GENERATED_BODY()
public:
	FHeroBaseStats();
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;
	
	UPROPERTY(EditAnywhere)
	float Strength;

	UPROPERTY(EditAnywhere)
	float Intelligence;

	UPROPERTY(EditAnywhere)
	float StrengthGrowRate;

	UPROPERTY(EditAnywhere)
	float IntelligenceGrowRate;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseMaxMana;

	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseAttackDamage;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
	
};
