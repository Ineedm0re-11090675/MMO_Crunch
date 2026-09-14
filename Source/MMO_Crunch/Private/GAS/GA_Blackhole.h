#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Blackhole.generated.h"

UCLASS()
class UGA_Blackhole : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	UPROPERTY(EditDefaultsOnly, Category="Target")
	float TargetAreaRadius =1000.f;

	UPROPERTY(EditDefaultsOnly, Category="Target")
	float BlackholePullSpeed =3000.f;

	UPROPERTY(EditDefaultsOnly, Category="Target")
	float BlackholeDuration =6.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Target")
	float TargetTraceRange =2000.f;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* TargetingMontage;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* HoldBlackholeMontage;


	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* FinalBlowMontage;

	
	UPROPERTY(EditDefaultsOnly, Category="Target")
	TSubclassOf<class ATargetActor_GroundPick> TargetActorClass;


	UPROPERTY(EditDefaultsOnly, Category="Target")
	TSubclassOf<class ATA_Blackhole> BlackholeTargetActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Target")
	TSubclassOf<UGameplayEffect> AimEffect;
	
	FActiveGameplayEffectHandle AimEffectHandle;


	UPROPERTY(EditDefaultsOnly, Category="Target")
	TSubclassOf<UGameplayEffect> FocusEffect;
	
	FActiveGameplayEffectHandle FocusEffectHandle;
	
	
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	TSubclassOf<UGameplayEffect> FinalBlowDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category="Effect")
	float BlowPushSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category="Effect")
	FGameplayTag FinalBlowCueTag;
	

	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* PlayCastBlackHoleMontageTask;

	UPROPERTY()
	class UAbilityTask_WaitTargetData* BlackholeTargetingTask;
	
	UFUNCTION()
	void PlaceBlackhole(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
	
	UFUNCTION()
	void PlacementCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
	
	UFUNCTION()
	void FinalTargetsReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	
	void AddAimEffect();
	void RemoveAimEffect();
	void AddFocusEffect();
	void RemoveFocusEffect();
};
