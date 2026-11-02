#include "GA_Blink.h"
#include "CAbilitySystemStatics.h"
#include "AbilitySystem/Effects/CombatEffect_Damage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include  "TargetActor_GroundPick.h"
#include "AbilitySystemBlueprintLibrary.h"
void UGA_Blink::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	if (!HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		K2_EndAbility();
		return;
	}
	UAbilityTask_PlayMontageAndWait* PlayTargetingMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,TargetingMontage);
	PlayTargetingMontage->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitBlinkLocationTargetData = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, GroundPickTargetActorClass);
	WaitBlinkLocationTargetData->ValidData.AddDynamic(this, &UGA_Blink::GroundPickTargetReceived);
	WaitBlinkLocationTargetData->Cancelled.AddDynamic(this, &UGA_Blink::GroundPickCancelled);
	WaitBlinkLocationTargetData->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitBlinkLocationTargetData->BeginSpawningActor(this, GroundPickTargetActorClass, TargetActor);
	ATargetActor_GroundPick* GroundPickTargetActor = Cast<ATargetActor_GroundPick>(TargetActor);
	if (GroundPickTargetActor)
	{
		GroundPickTargetActor->SetShouldDrawDebug(ShouldDrawDebugSphere());
		GroundPickTargetActor->SetTargetAreaRadius(TargetAreaRadius);
		GroundPickTargetActor->SetTargetTraceRange(BlinkCastRange);
	}

	WaitBlinkLocationTargetData->FinishSpawningActor(this, TargetActor);
	
}

UGA_Blink::UGA_Blink()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimingStatsAbilityTag());
}

FGameplayTag UGA_Blink::GetTeleportationTag()
{
	return FGameplayTag::RequestGameplayTag("ability.blink.teleport");
}

void UGA_Blink::GroundPickTargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{if (!K2_CommitAbility())
{
	K2_EndAbility();
	return;
}

	BlinkTargetDataHandle = TargetDataHandle;

	if (HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayTeleportMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, TeleportMontage);
		PlayTeleportMontage->OnBlendOut.AddDynamic(this, &UGA_Blink::K2_EndAbility);
		PlayTeleportMontage->OnCancelled.AddDynamic(this, &UGA_Blink::K2_EndAbility);
		PlayTeleportMontage->OnInterrupted.AddDynamic(this, &UGA_Blink::K2_EndAbility);
		PlayTeleportMontage->OnCompleted.AddDynamic(this, &UGA_Blink::K2_EndAbility);
		PlayTeleportMontage->ReadyForActivation();

		if (K2_HasAuthority())
		{
			UAbilityTask_WaitGameplayEvent* WaitTeleportTimepoint = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetTeleportationTag());
			WaitTeleportTimepoint->EventReceived.AddDynamic(this, &UGA_Blink::Teleport);
			WaitTeleportTimepoint->ReadyForActivation();
		}
	}
}


void UGA_Blink::GroundPickCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}

void UGA_Blink::Teleport(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		FHitResult PickedLocationHitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(BlinkTargetDataHandle, 1);
		FVector PickedTeleportLocation = PickedLocationHitResult.ImpactPoint;

		GetAvatarActorFromActorInfo()->SetActorLocation(PickedTeleportLocation);
		// Codex: Route configured Blink damage through MapleCombat; zero preserves the current empty-GE behavior.
		ApplyMapleCombatDamageToTargetData(BlinkTargetDataHandle, UCombatEffect_Damage::StaticClass(), DamageAmount,
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		PushTargetsFromOwnerLocation(BlinkTargetDataHandle, BlinkLandTargetPushSpeed);
	}
}
