#include "GAP_Launched.h"

#include "CAbilitySystemStatics.h"

UGAP_Launched::UGAP_Launched()
{
	NetExecutionPolicy =EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FAbilityTriggerData TriggerEventData;
	TriggerEventData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerEventData.TriggerTag = GetLaunchedAbilityActivationTag();

	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatsAbilityTag()); 
	AbilityTriggers.Add(TriggerEventData);
}

void UGAP_Launched::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	if (K2_HasAuthority())
	{
		//可以将数据存在这个变量中
		PushSelf(TriggerEventData->TargetData.Get(0)->GetHitResult()->ImpactNormal);
		K2_EndAbility();
	}
}

FGameplayTag UGAP_Launched::GetLaunchedAbilityActivationTag()
{
	return FGameplayTag::RequestGameplayTag("ability.passive.launch.activate") ;
}
