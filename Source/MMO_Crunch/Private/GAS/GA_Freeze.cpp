#include "GA_Freeze.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/CombatEffect_Damage.h"
#include "CAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "TargetActor_GroundPick.h"
UGA_Freeze::UGA_Freeze()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimingStatsAbilityTag());
}

void UGA_Freeze::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(CurrentActorInfo,&CurrentActivationInfo))
	{
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayTargetingMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,TargetingMontage);
	PlayTargetingMontage->OnBlendOut.AddDynamic(this,&UGA_Freeze::K2_EndAbility);
	PlayTargetingMontage->OnCancelled.AddDynamic(this,&UGA_Freeze::K2_EndAbility);
	PlayTargetingMontage->OnCompleted.AddDynamic(this,&UGA_Freeze::K2_EndAbility);
	PlayTargetingMontage->OnInterrupted.AddDynamic(this,&UGA_Freeze::K2_EndAbility);
	PlayTargetingMontage->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetActorClass);
	WaitTargetData->ValidData.AddDynamic(this,&UGA_Freeze::TargetReceived);
	WaitTargetData->Cancelled.AddDynamic(this,&UGA_Freeze::TargetingCancelled);
	WaitTargetData->ReadyForActivation();
	
	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetData->BeginSpawningActor(this,TargetActorClass,TargetActor);
	ATargetActor_GroundPick* TargetActor_GroundPick = Cast<ATargetActor_GroundPick>(TargetActor);

	if (TargetActor_GroundPick)
	{
		TargetActor_GroundPick->SetShouldDrawDebug(ShouldDrawDebugSphere());
		TargetActor_GroundPick->SetTargetAreaRadius(TargetingRadius);
		TargetActor_GroundPick->SetTargetTraceRange(TargetRange);
	}
	WaitTargetData->FinishSpawningActor(this,TargetActor_GroundPick);
}

void UGA_Freeze::TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	FHitResult PlacementHit;
	if (!ATargetActor_GroundPick::GetPlacementHitResult(TargetDataHandle, PlacementHit))
	{
		// Codex: End cleanly when malformed target data has no appended GroundPick placement hit.
		K2_EndAbility();
		return;
	}

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	if (K2_HasAuthority())
	{
		// Codex: Route configured Freeze damage through MapleCombat; zero preserves the current empty-GE behavior.
		ApplyMapleCombatDamageToTargetData(TargetDataHandle, UCombatEffect_Damage::StaticClass(), DamageAmount,
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}

	FGameplayCueParameters FreezeCueParameters;
	// Codex: GroundPick writes its selected point to ImpactPoint and appends it after all target actors.
	FreezeCueParameters.Location = PlacementHit.ImpactPoint;
	FreezeCueParameters.RawMagnitude = TargetingRadius;

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(FreezingGameplayCueTag,FreezeCueParameters);

	PlayMontageLocally(CastMontage);
	K2_EndAbility();
}

void UGA_Freeze::TargetingCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}
