#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Lazer.generated.h"

class AGameplayAbilityTargetActor;

UCLASS()
class UGA_Lazer : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	static FGameplayTag GetShootTag();
private:
	UPROPERTY(EditDefaultsOnly,Category ="Targeting")
	float TargetRange = 4000;
	
	UPROPERTY(EditDefaultsOnly,Category ="Targeting")
	float DetectionCylinderRadius = 30;
	
	UPROPERTY(EditDefaultsOnly,Category ="Targeting")
	float TargetingInterval = 0.3f;

		
	UPROPERTY(EditDefaultsOnly,Category = "Effect")
	TSubclassOf<UGameplayEffect> HitDamageEffect;
		
	UPROPERTY(EditDefaultsOnly,Category = "Effect")
	float HitPushSpeed = 3000;
	
	UPROPERTY(EditDefaultsOnly,Category = "Effect")
	TSubclassOf<UGameplayEffect> OnGoingConsumingEffect;

	
	FActiveGameplayEffectHandle OnGoingConsumingEffectHandle;
	
	UPROPERTY(EditDefaultsOnly,Category ="Anim")
	UAnimMontage* LazerMontage;

	UPROPERTY(EditDefaultsOnly,Category ="Targeting")
	TSubclassOf<AGameplayAbilityTargetActor> LazerTAClass;
	
	UPROPERTY(EditDefaultsOnly,Category ="Targeting")
	FName TAAttachSocketName = "Lazer";
	UFUNCTION() 
	void ShootLazer(FGameplayEventData Payload);

	void ManaUpdated(const FOnAttributeChangeData& Payload);

	UFUNCTION()
	void TargetReceived(const FGameplayAbilityTargetDataHandle& TargetData);
};
