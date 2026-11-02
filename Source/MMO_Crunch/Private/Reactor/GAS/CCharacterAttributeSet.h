#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MapleGASAttributeSet.h"
#include "CCharacterAttributeSet.generated.h"

UCLASS()
class MMO_CRUNCH_API UCCharacterAttributeSet : public UMapleGASAttributeSet
{
	GENERATED_BODY()
public:
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, Level)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, Experience)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, Gold)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, UpgradePoint)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, MaxLevel)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, MaxLevelExperience)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, PrevLevelExperience)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, NextLevelExperience)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, Strength)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, Intelligence)
	// Codex: Preserve both Crunch progression growth-rate attributes from the former Hero Attribute Set.
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, StrengthGrowRate)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, IntelligenceGrowRate)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, Armor)
	// Codex: Carry legacy base AttackDamage into MapleGAS for MapleCombat SetByCaller damage.
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, AttackDamage)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, MoveSpeed)
	ATTRIBUTE_ACCESSORS(UCCharacterAttributeSet, MoveAcceleration)

	// Codex: Register Crunch attributes for network replication through MapleGAS-safe RepNotify callbacks.
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
private:
	// Codex: ReplicatedUsing keeps clients' GAS attribute aggregators in sync with server values.
	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression",meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_Level)
	FGameplayAttributeData Level;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_Experience)
	FGameplayAttributeData Experience;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_Gold)
	FGameplayAttributeData Gold;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_UpgradePoint)
	FGameplayAttributeData UpgradePoint;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_MaxLevel)
	FGameplayAttributeData MaxLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_MaxLevelExperience)
	FGameplayAttributeData MaxLevelExperience;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_PrevLevelExperience)
	FGameplayAttributeData PrevLevelExperience;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_NextLevelExperience)
	FGameplayAttributeData NextLevelExperience;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Stats", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Stats", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_Intelligence)
	FGameplayAttributeData Intelligence;

	// Codex: These remain Crunch-owned stats; MapleCombat ArmorReduction has different semantics.
	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Stats",meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;

	// Codex: Replace UCAttributeSet.AttackDamage as the source value for Crunch combo damage.
	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Stats", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Movement", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;

	// Codex: Retain the legacy growth-rate fields and replicate them for clients that inspect character progression.
	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_StrengthGrowRate)
	FGameplayAttributeData StrengthGrowRate;

	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Progression", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_IntelligenceGrowRate)
	FGameplayAttributeData IntelligenceGrowRate;

	// Codex: Kept because it is part of the requested migration inventory; its runtime consumer still needs confirmation.
	UPROPERTY(BlueprintReadOnly, Category = "Crunch|Movement", meta = (AllowPrivateAccess = "true"),ReplicatedUsing = OnRep_MoveAcceleration)
	FGameplayAttributeData MoveAcceleration;

	// Codex: RepNotify declarations use MapleGAS's lazy-safe attribute notification macro in the .cpp.
	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Experience(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Gold(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_UpgradePoint(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxLevel(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxLevelExperience(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_PrevLevelExperience(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_NextLevelExperience(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StrengthGrowRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_IntelligenceGrowRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveAcceleration(const FGameplayAttributeData& OldValue);
};
