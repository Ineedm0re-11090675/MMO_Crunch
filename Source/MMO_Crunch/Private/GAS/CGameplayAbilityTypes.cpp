#include "CGameplayAbilityTypes.h"

FGenericDamageEffectDef::FGenericDamageEffectDef()
	:DamageEffect{nullptr},
	PushVelocity{0.f},
	// Codex: Existing Crunch damage GEs use a -1 Health modifier; preserve that as 1 positive MapleCombat damage point.
	DamageAmount{1.0f}
{ 
}

FHeroBaseStats::FHeroBaseStats()
	: Class{nullptr},
	  Strength{0.f},
	  Intelligence{0.f},
	  StrengthGrowRate{0.f},
	  IntelligenceGrowRate{0.f},
	  BaseMaxHealth{0.f},
	  BaseMaxMana{0.f},
	  BaseArmor{0.f},
	  BaseAttackDamage{0.f},
	  BaseMoveSpeed{0.f}
{
}
