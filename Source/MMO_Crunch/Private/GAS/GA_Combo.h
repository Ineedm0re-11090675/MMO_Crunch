#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
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

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;
	
	UPROPERTY(EditDefaultsOnly,Category = "Animation")
	UAnimMontage* ComboMontage;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effect")
	TMap<FName,TSubclassOf<UGameplayEffect>> DamageEffectsMap;
	UPROPERTY(EditDefaultsOnly,Category = "Targetting")
	float TargetSweepSphereRadius =30.f;
	TSubclassOf<UGameplayEffect> GetCurrentComboDamageEffect() const;
	
	UFUNCTION()
	void ComboEventChangedReceived(FGameplayEventData Data);

	UFUNCTION()
	void HandleInputPress(float TimeWaited);

	UFUNCTION()
	void HandleTargetGroupReceive(FGameplayEventData Data);
	void TryCommitCombo();
	FName NextComboName;
};
