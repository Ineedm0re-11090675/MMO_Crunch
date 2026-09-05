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

bool UCAbilitySystemStatics::IsAbilityAtMaxLevel(const FGameplayAbilitySpec& AbilitySpec)
{ 
	return AbilitySpec.Level  >= 4;
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

bool UCAbilitySystemStatics::CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec,
	const UAbilitySystemComponent& AbilitySystemComponent)
{
	//
	//查看当前蓝是否支持释放技能
	//
	const UGameplayAbility* AbilityCOD = AbilitySpec.Ability;
	if (AbilityCOD)
	{
		return AbilityCOD->CheckCost(AbilitySpec.Handle,AbilitySystemComponent.AbilityActorInfo.Get());
	}
	 return false;
}

float UCAbilitySystemStatics::GetManaCostFor(const UGameplayAbility* AbilityCOD, const UAbilitySystemComponent& OwnASC,
	int Level)
{
	//根据Level查耗蓝量，然后更新在AbilityGauge
	/*
		*Ability
		↓
		找到 Cost GE
		↓
		按指定 Level 创建 GE Spec
		↓
		计算 Modifier[0] 的 Magnitude
		↓
		得到 -50
		↓
		Abs
		↓
		返回 50
	*/
	float ManaCost = 0.f;
	if (AbilityCOD)
	{
		UGameplayEffect* CostEffect =AbilityCOD->GetCostGameplayEffect();
		if (CostEffect)
		{
			FGameplayEffectSpecHandle EffectHandle = OwnASC.MakeOutgoingSpec(CostEffect->GetClass(),Level,OwnASC.MakeEffectContext());
			CostEffect->Modifiers[0].ModifierMagnitude.AttemptCalculateMagnitude(* EffectHandle.Data.Get(),ManaCost);
		}
	}
	return FMath::Abs(ManaCost);
}

float UCAbilitySystemStatics::GetCooldownDurationFor(const UGameplayAbility* AbilityCOD, const UAbilitySystemComponent& OwnASC,
	int Level)
{
	float Cooldown = 0.f;
	if (AbilityCOD)
	{
		UGameplayEffect* CooldownEffect =AbilityCOD->GetCooldownGameplayEffect();
		if (CooldownEffect)
		{
			FGameplayEffectSpecHandle EffectHandle = OwnASC.MakeOutgoingSpec(CooldownEffect->GetClass(),Level,OwnASC.MakeEffectContext());
			CooldownEffect->DurationMagnitude.AttemptCalculateMagnitude(* EffectHandle.Data.Get(),Cooldown );
		}
	}
	return FMath::Abs(Cooldown);
}

float UCAbilitySystemStatics::GetCooldownRemainingFor(const UGameplayAbility* AbilityCOD,
	const UAbilitySystemComponent& OwnASC)
{
	if (!AbilityCOD) return 0.f;
	UGameplayEffect* CooldownEffect =AbilityCOD->GetCooldownGameplayEffect();
	if (!CooldownEffect) return 0.f;
	FGameplayEffectQuery CooldownEffectQuery;
	CooldownEffectQuery.EffectDefinition = CooldownEffect->GetClass();

	float CooldownRemaining = 0.f;
	FJsonSerializableArrayFloat CooldownRemainings = OwnASC.GetActiveEffectsTimeRemaining(CooldownEffectQuery);

	for (float Remaining : CooldownRemainings)
	{
		if (Remaining > CooldownRemaining)
		{
			CooldownRemaining = Remaining;
		}
	}
	return FMath::Abs(CooldownRemaining); 
}
