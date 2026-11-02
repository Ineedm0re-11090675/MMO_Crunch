#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Freeze.generated.h"


UCLASS()
class UGA_Freeze : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Freeze();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetingRadius = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetRange = 2000.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Anim")
	UAnimMontage* TargetingMontage;
	
	UPROPERTY(EditDefaultsOnly,Category = "Anim")
	UAnimMontage* CastMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ATargetActor_GroundPick> TargetActorClass;

	// Codex: The current Freeze blueprint has no damage GE, so preserve its no-damage default until configured.
	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta = (ClampMin = "0.0"))
	float DamageAmount = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayCue")
	FGameplayTag FreezingTargetGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayCue")
	FGameplayTag FreezingGameplayCueTag;
	

	UFUNCTION()
	void TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void TargetingCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
};
