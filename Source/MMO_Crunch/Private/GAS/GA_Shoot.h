#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Shoot.generated.h"

UCLASS()
class UGA_Shoot : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Shoot();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	UPROPERTY(EditDefaultsOnly,Category = "Shoot")
	TSubclassOf<UGameplayEffect> ProjectileHitEffect;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shoot")
	float ShootProjectileSpeed = 2000;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shoot")
	float ShootProjectileRange = 3000;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shoot")
	TSubclassOf<class AProjectileActor> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly,Category = "Anim")
	UAnimMontage* ShootMontage;
	
	static FGameplayTag GetShootTag();
	
	UFUNCTION()
	void StartShooting(FGameplayEventData Payload);
	
	UFUNCTION()
	void StopShooting(FGameplayEventData Payload);
	
	UFUNCTION()
	void ShootProjectile(FGameplayEventData Payload);

	AActor* GetAimTargetIfValid() const;


	UPROPERTY()
	AActor* AimTarget;
	UPROPERTY()
	UAbilitySystemComponent* AimTargetASC;

	FTimerHandle AimTargetCheckTimerHandle;

	void FindAimTarget();

	UPROPERTY(EditDefaultsOnly,Category = "Target")
	float AimTargetCheckTimeInterval = 0.1f;

	void StartAimTargetCheckTimer();
	void StopAimTargetCheckTimer();

	bool HasValidTarget() const;
	bool IsActorInRange() const;

	void TargetDeadTagUpdate(const FGameplayTag Tag,int32 NewCount);
};
