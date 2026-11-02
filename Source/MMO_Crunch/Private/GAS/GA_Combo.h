#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "AbilitySystem/Abilities/CombatAbility_Attack.h"
#include "AbilitySystem/Abilities/CombatAbility_MeleeAttack.h"
#include "GA_Combo.generated.h"

UCLASS()
class UGA_Combo : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Combo();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	static FGameplayTag GetComboEventTChangedTag();
	static FGameplayTag GetComboEventEndTag();
	static FGameplayTag GetComboTargetGroupTag();
private:
	void SetupWaitInputPress();

	// Codex: Preserve the old MMC's source AttackDamage input; MapleCombat applies target-side defense.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect", meta = (ClampMin = "0.0"))
	float DefaultDamageMultiplier = 1.0f;
	// Codex: All four legacy combo GEs use the same MMC, so each section defaults to a 1.0 multiplier.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TMap<FName, float> DamageMultipliersMap;
	
	UPROPERTY(EditDefaultsOnly,Category = "Animation")
	UAnimMontage* ComboMontage;

	// Codex: Read source AttackDamage from MapleGAS and scale it for the active montage section.
	float GetCurrentComboDamageAmount() const;
	float GetCurrentComboDamageMultiplier() const;
	
	UFUNCTION()
	void ComboEventChangedReceived(FGameplayEventData Data);

	UFUNCTION()
	void HandleInputPress(float TimeWaited);

	UFUNCTION()
	void HandleTargetGroupReceive(FGameplayEventData Data);
	void TryCommitCombo();
	FName NextComboName;
};
