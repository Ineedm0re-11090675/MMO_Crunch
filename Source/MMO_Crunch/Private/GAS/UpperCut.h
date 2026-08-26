#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "CGameplayAbilityTypes.h"
#include "UpperCut.generated.h"

UCLASS()
class UUpperCut : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UUpperCut();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UPROPERTY(EditDefaultsOnly,Category = "Combo")
	TMap<FName,FGenericDamageEffectDef>DamageMap;

	
	UPROPERTY(EditDefaultsOnly,Category = "Animation")
	UAnimMontage* UpperCutMontage;

	UPROPERTY(EditDefaultsOnly,Category = "Launch")
	TSubclassOf<UGameplayEffect> LaunchDamageEffect;
	UPROPERTY(EditDefaultsOnly,Category = "Launch")
	float UppercutLaunchSpeed = 1000.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Launch")
	float UppercutComboHoldSpeed= 100.f;
	
	static FGameplayTag GetUpperCutTag();

	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);


	UPROPERTY(EditDefaultsOnly,Category = "Targetting")
	float TargetSweepSphereRadius =30.f;

	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData Data);

	UFUNCTION()
	void HandleComboCommitEvent(FGameplayEventData Data);

	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData Data);
	
	FName NextComboName;

	const FGenericDamageEffectDef* GetDamageDefFromCurrentCombo() const ;
};
