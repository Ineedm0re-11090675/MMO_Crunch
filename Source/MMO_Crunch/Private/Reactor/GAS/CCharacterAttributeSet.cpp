#include "CCharacterAttributeSet.h"

#include "Net/UnrealNetwork.h"

// Codex: Replicate Crunch-owned attributes so clients can observe server updates.
void UCCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, Experience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, Gold, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, UpgradePoint, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, MaxLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, MaxLevelExperience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, PrevLevelExperience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, NextLevelExperience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, StrengthGrowRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, IntelligenceGrowRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	// Codex: Replicate the migrated attack power used by MapleCombat combo damage.
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCharacterAttributeSet, MoveAcceleration, COND_None, REPNOTIFY_Always);
}

// Codex: Use MapleGAS's lazy-safe RepNotify path for attributes replicated before ASC initialization.
void UCCharacterAttributeSet::OnRep_Level(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, Level, OldValue);
}

void UCCharacterAttributeSet::OnRep_Experience(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, Experience, OldValue);
}

void UCCharacterAttributeSet::OnRep_Gold(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, Gold, OldValue);
}

void UCCharacterAttributeSet::OnRep_UpgradePoint(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, UpgradePoint, OldValue);
}

void UCCharacterAttributeSet::OnRep_MaxLevel(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, MaxLevel, OldValue);
}

void UCCharacterAttributeSet::OnRep_MaxLevelExperience(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, MaxLevelExperience, OldValue);
}

void UCCharacterAttributeSet::OnRep_PrevLevelExperience(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, PrevLevelExperience, OldValue);
}

void UCCharacterAttributeSet::OnRep_NextLevelExperience(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, NextLevelExperience, OldValue);
}

void UCCharacterAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, Strength, OldValue);
}

void UCCharacterAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, Intelligence, OldValue);
}

void UCCharacterAttributeSet::OnRep_StrengthGrowRate(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, StrengthGrowRate, OldValue);
}

void UCCharacterAttributeSet::OnRep_IntelligenceGrowRate(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, IntelligenceGrowRate, OldValue);
}

void UCCharacterAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, Armor, OldValue);
}

// Codex: Use MapleGAS's lazy-safe RepNotify path for migrated attack power.
void UCCharacterAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, AttackDamage, OldValue);
}

void UCCharacterAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, MoveSpeed, OldValue);
}

void UCCharacterAttributeSet::OnRep_MoveAcceleration(const FGameplayAttributeData& OldValue)
{
	LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UCCharacterAttributeSet, MoveAcceleration, OldValue);
}
