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
	// Codex: Reject invalid launch payloads before committing the passive ability or reading TargetData.
	if (!K2_HasAuthority() || !TriggerEventData || TriggerEventData->TargetData.Num() == 0)
	{
		K2_EndAbility();
		return;
	}

	const FGameplayAbilityTargetData* LaunchTargetData = TriggerEventData->TargetData.Get(0);
	const FHitResult* LaunchHitResult = LaunchTargetData ? LaunchTargetData->GetHitResult() : nullptr;
	if (!LaunchHitResult)
	{
		K2_EndAbility();
		return;
	}

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	PushSelf(LaunchHitResult->ImpactNormal);
	K2_EndAbility();
}

FGameplayTag UGAP_Launched::GetLaunchedAbilityActivationTag()
{
	return FGameplayTag::RequestGameplayTag("ability.passive.launch.activate") ;
}
