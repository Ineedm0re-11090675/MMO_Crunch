#include "GA_Tornado.h"

#include "CAbilitySystemStatics.h"
#include "AbilitySystem/Effects/CombatEffect_Damage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
void UGA_Tornado::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayTornadoMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,TornadoMontage);
		PlayTornadoMontage->OnBlendOut.AddDynamic(this,&UGA_Tornado::K2_EndAbility);
		PlayTornadoMontage->OnInterrupted.AddDynamic(this,&UGA_Tornado::K2_EndAbility);
		PlayTornadoMontage->OnCancelled.AddDynamic(this,&UGA_Tornado::K2_EndAbility);
		PlayTornadoMontage->OnCompleted.AddDynamic(this,&UGA_Tornado::K2_EndAbility);
		PlayTornadoMontage->ReadyForActivation();

		if (K2_HasAuthority())
		{
			UAbilityTask_WaitGameplayEvent* WaitDamageEvent =UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetGenericDamagePointTag());
			WaitDamageEvent->EventReceived.AddDynamic(this,&UGA_Tornado::TornadoDamageEventReceived);
			WaitDamageEvent->ReadyForActivation();
		}

		UAbilityTask_WaitCancel* WaitCancel = UAbilityTask_WaitCancel::WaitCancel(this);
		WaitCancel->OnCancel.AddDynamic(this,&UGA_Tornado::K2_EndAbility);
		WaitCancel->ReadyForActivation();


		UAbilityTask_WaitDelay* WaitDelay = UAbilityTask_WaitDelay::WaitDelay(this,TornadoDuration);
		WaitDelay->OnFinish.AddDynamic(this,&UGA_Tornado::K2_EndAbility);
		WaitDelay->ReadyForActivation();
	}
}

void UGA_Tornado::TornadoDamageEventReceived(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle = Payload.TargetData;
		// Codex: Route future Tornado damage through MapleCombat; zero keeps the current empty-GE behavior.
		ApplyMapleCombatDamageToTargetData(TargetDataHandle, UCombatEffect_Damage::StaticClass(), HitDamageAmount,
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		PushTargetsFromOwnerLocation(TargetDataHandle,HitPushSpeed);
	}
}
