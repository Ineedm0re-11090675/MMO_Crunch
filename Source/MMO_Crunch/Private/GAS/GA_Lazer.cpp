#include "GA_Lazer.h"

#include "AbilitySystemComponent.h"
#include "CAttributeSet.h"
#include "TargetActor_Line.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

void UGA_Lazer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !LazerMontage)
	{
		K2_EndAbility();
		return;
	}
	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayLazerMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,LazerMontage);
		PlayLazerMontageTask->OnBlendOut.AddDynamic(this,&UGA_Lazer::K2_EndAbility);
		PlayLazerMontageTask->OnInterrupted.AddDynamic(this,&UGA_Lazer::K2_EndAbility);
		PlayLazerMontageTask->OnCancelled.AddDynamic(this,&UGA_Lazer::K2_EndAbility);
		PlayLazerMontageTask->OnCompleted.AddDynamic(this,&UGA_Lazer::K2_EndAbility);
		PlayLazerMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetShootTag());
		WaitShootEvent->EventReceived.AddDynamic(this,&UGA_Lazer::ShootLazer);
		WaitShootEvent->ReadyForActivation();

		UAbilityTask_WaitCancel* WaitShootCancel = UAbilityTask_WaitCancel::WaitCancel(this);
		WaitShootCancel->OnCancel.AddDynamic(this,&UGA_Lazer::K2_EndAbility);
		WaitShootCancel->ReadyForActivation();
	}
}

void UGA_Lazer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (OwnerASC && OnGoingConsumingEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(OnGoingConsumingEffectHandle);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_Lazer::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.lazer.shoot");
}

void UGA_Lazer::ShootLazer(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		OnGoingConsumingEffectHandle = BP_ApplyGameplayEffectToOwner(OnGoingConsumingEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
		if (OwnerASC)
		{
			OwnerASC->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this,&UGA_Lazer::ManaUpdated);
		}
	}
	UAbilityTask_WaitTargetData* WaitDamageTargetTask =UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::CustomMulti,LazerTAClass);
	WaitDamageTargetTask->ValidData.AddDynamic(this,&UGA_Lazer::TargetReceived);
	WaitDamageTargetTask->ReadyForActivation();
	
	AGameplayAbilityTargetActor* TargetActor;
	WaitDamageTargetTask->BeginSpawningActor(this,LazerTAClass,TargetActor);
	ATargetActor_Line* LineTargetActor = Cast<ATargetActor_Line>(TargetActor);
	
	if (LineTargetActor)
	{
		LineTargetActor->ConfigureTargetSetting(TargetRange,DetectionCylinderRadius,TargetingInterval,GetOwnerTeamId(),ShouldDrawDebugSphere());
	}
	WaitDamageTargetTask->FinishSpawningActor(this,TargetActor);
	if (LineTargetActor)
	{
		LineTargetActor->AttachToComponent(GetOwningComponentFromActorInfo(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,TAAttachSocketName);
	}
}

void UGA_Lazer::ManaUpdated(const FOnAttributeChangeData& Payload)
{
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (OwnerASC && !OwnerASC->CanApplyAttributeModifiers(OnGoingConsumingEffect.GetDefaultObject(),GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo),MakeEffectContext(CurrentSpecHandle,CurrentActorInfo)))
	{
		K2_EndAbility();
	}
}

void UGA_Lazer::TargetReceived(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(TargetData,HitDamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
	}
	PushTargets(TargetData,GetAvatarActorFromActorInfo()->GetActorForwardVector() * HitPushSpeed);
}

