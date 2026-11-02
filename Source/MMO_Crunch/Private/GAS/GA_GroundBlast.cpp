#include "GA_GroundBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAbilitySystemStatics.h"

// Codex: Use the native MapleCombat damage execution for the migrated Ground Blast path.
#include "AbilitySystem/Effects/CombatEffect_Damage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TargetActor_GroundPick.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
UGA_GroundBlast::UGA_GroundBlast()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimingStatsAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(CurrentActorInfo,&CurrentActivationInfo)) return;


	UAbilityTask_PlayMontageAndWait* PlayGroundBlastMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,GroundBlastMontage);

	PlayGroundBlastMontage->OnBlendOut.AddDynamic(this,&UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastMontage->OnCancelled.AddDynamic(this,&UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastMontage->OnCompleted.AddDynamic(this,&UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastMontage->OnInterrupted  .AddDynamic(this,&UGA_GroundBlast::K2_EndAbility);	
	PlayGroundBlastMontage->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this,&UGA_GroundBlast::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this,&UGA_GroundBlast::TargetCanceled);

	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask->BeginSpawningActor(this,TargetActorClass,TargetActor);
	
	ATargetActor_GroundPick* TargetActorGroundPick =Cast<ATargetActor_GroundPick>(TargetActor);
	if (TargetActorGroundPick)
	{
		TargetActorGroundPick->SetShouldDrawDebug(ShouldDrawDebugSphere());
		TargetActorGroundPick->SetTargetAreaRadius(TargetAreaRadius);
		TargetActorGroundPick->SetTargetTraceRange(TargetTraceRange);
	}
	
	WaitTargetDataTask->FinishSpawningActor(this,TargetActor);
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	} 
	
	if (K2_HasAuthority())
	{
		// Codex: The former -1 Health GE maps to 1 positive MapleCombat damage point.
		ApplyMapleCombatDamageToTargetData(Data, UCombatEffect_Damage::StaticClass(),
			GroundBlastDamage.DamageAmount, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		PushTargets(Data,GroundBlastDamage.PushVelocity);
	}
	FGameplayCueParameters Parameters;
	Parameters.Location = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Data,1).ImpactPoint;
	Parameters.RawMagnitude = TargetAreaRadius;

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BlastGameplayCueTag,Parameters);
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(UCAbilitySystemStatics::GetCameraShakeGameplayCueTag(),Parameters);
	
	UAnimInstance* OwnInstance = GetOwnerAnimInstance();
	if (OwnInstance)
	{
		OwnInstance->Montage_Play(CastMontage);
	}
	
	K2_EndAbility();
}

void UGA_GroundBlast::TargetCanceled(const FGameplayAbilityTargetDataHandle& Data)
{
#if WITH_EDITOR
	UE_LOG(LogTemp,Warning,TEXT("Cancel"))
#endif
	K2_EndAbility();
}
