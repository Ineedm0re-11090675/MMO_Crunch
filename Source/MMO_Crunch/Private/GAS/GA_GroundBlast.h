#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "CGameplayAbilityTypes.h"
#include "GA_GroundBlast.generated.h"

UCLASS()
class UGA_GroundBlast : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_GroundBlast();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* GroundBlastMontage;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* CastMontage;

	UPROPERTY(EditDefaultsOnly, Category="Cue")
	FGameplayTag BlastGameplayCueTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Target")
	TSubclassOf<class ATargetActor_GroundPick> TargetActorClass ;	

	UPROPERTY(EditDefaultsOnly, Category="Target")
	float TargetAreaRadius =300.f;

	UPROPERTY(EditDefaultsOnly, Category="Target")
	float TargetTraceRange =2000.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGenericDamageEffectDef GroundBlastDamage;

	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCanceled(const FGameplayAbilityTargetDataHandle& Data);
};
