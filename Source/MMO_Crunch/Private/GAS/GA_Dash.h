#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Dash.generated.h"

UCLASS()
class UGA_Dash : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	static FGameplayTag GetDashStartTag();
private:
	UPROPERTY(EditDefaultsOnly,Category = "Montage")
	UAnimMontage* DashMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffect;
	// Codex: Supply the magnitude expected by MapleCombat's Combat.Data.Damage SetByCaller execution.
	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (ClampMin = "0.0"))
	float DamageAmount = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DashEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ATargetActor_Around> TargetActorClass;
	
	UFUNCTION()
	void StartDash(FGameplayEventData Payload);

	UFUNCTION()
	void TargetReceive(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UPROPERTY()
	class UCharacterMovementComponent* OwnCharacterMovementComponent;

	FActiveGameplayEffectHandle DashEffectHandle;

	FTimerHandle PushForwardInputTimerHandle;

	void PushForward();

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float TargetHitPushSpeed = 3000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetDetectionRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayCue")
	FGameplayTag LocalGameplayCueTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	FName TargetActorAttachSocketName = "TargetDashCenter";
};
