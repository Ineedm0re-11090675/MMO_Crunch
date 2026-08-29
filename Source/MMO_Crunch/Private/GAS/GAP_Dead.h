#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GAP_Dead.generated.h"


UCLASS()
class UGAP_Dead : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGAP_Dead();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UPROPERTY(EditDefaultsOnly,Category ="Reward")
	float RewardRange = 1000.f;

	TArray<AActor*> GetRewardActors() const ;

	UPROPERTY(EditDefaultsOnly,Category ="Reward")
	float BaseExperienceReward = 200.f;
	
	UPROPERTY(EditDefaultsOnly,Category ="Reward")
	float BaseGoldReward = 200.f;


	UPROPERTY(EditDefaultsOnly,Category ="Reward")
	float ExperienceRewardPerExp = 0.1f;


	UPROPERTY(EditDefaultsOnly,Category ="Reward")
	float GoldRewardPerExp =  0.05f;

	UPROPERTY(EditDefaultsOnly,Category ="Reward")
	float KillerRewardPortion = 0.5f;
	
	UPROPERTY(EditDefaultsOnly,Category ="Reward")
	TSubclassOf<UGameplayEffect> RewardEffect;
};
