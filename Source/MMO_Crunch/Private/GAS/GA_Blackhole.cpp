#include "GA_Blackhole.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAbilitySystemStatics.h"
#include "TargetActor_GroundPick.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "TA_Blackhole.h"
void UGA_Blackhole::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		K2_EndAbility();
		return;
	}

	PlayCastBlackHoleMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,TargetingMontage);
	PlayCastBlackHoleMontageTask->OnBlendOut.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
	PlayCastBlackHoleMontageTask->OnCancelled.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
	PlayCastBlackHoleMontageTask->OnCompleted.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
	PlayCastBlackHoleMontageTask->OnInterrupted.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
	PlayCastBlackHoleMontageTask->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitPlacementTask =UAbilityTask_WaitTargetData::WaitTargetData(
		this,
		NAME_None,EGameplayTargetingConfirmation::UserConfirmed,
		TargetActorClass
		);
	WaitPlacementTask->ValidData.AddDynamic(this,&UGA_Blackhole::PlaceBlackhole);
	WaitPlacementTask->Cancelled.AddDynamic(this,&UGA_Blackhole::PlacementCancelled );
	WaitPlacementTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitPlacementTask->BeginSpawningActor(this,TargetActorClass,TargetActor);
	ATargetActor_GroundPick* TargetActor_GroundPick = Cast<ATargetActor_GroundPick>(TargetActor);
	if (TargetActor_GroundPick)
	{
		TargetActor_GroundPick->SetShouldDrawDebug(ShouldDrawDebugSphere());
		TargetActor_GroundPick->SetTargetAreaRadius(TargetAreaRadius);
		TargetActor_GroundPick->SetTargetTraceRange(TargetTraceRange);
	}
	WaitPlacementTask->FinishSpawningActor(this,TargetActor);
	AddAimEffect();
}

void UGA_Blackhole::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveAimEffect();
	RemoveFocusEffect();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled); 
}

void UGA_Blackhole::PlaceBlackhole(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if(!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	RemoveAimEffect();
	AddFocusEffect();
	
	if (PlayCastBlackHoleMontageTask)
	{
		PlayCastBlackHoleMontageTask->OnBlendOut.RemoveAll(this);
		PlayCastBlackHoleMontageTask->OnCancelled.RemoveAll(this);
		PlayCastBlackHoleMontageTask->OnCompleted.RemoveAll(this);
		PlayCastBlackHoleMontageTask->OnInterrupted.RemoveAll(this);
		PlayCastBlackHoleMontageTask->ReadyForActivation();
	}
	if (HasAuthorityOrPredictionKey(CurrentActorInfo,&CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayHoldBlackholeMontageTask =UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,HoldBlackholeMontage); 

		PlayHoldBlackholeMontageTask->OnBlendOut.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
		PlayHoldBlackholeMontageTask->OnCancelled.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
		PlayHoldBlackholeMontageTask->OnCompleted.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
		PlayHoldBlackholeMontageTask->OnInterrupted.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
		
		PlayHoldBlackholeMontageTask->ReadyForActivation(); 
	}
	BlackholeTargetingTask = UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::UserConfirmed,BlackholeTargetActorClass);
	BlackholeTargetingTask->ValidData.AddDynamic(this,&UGA_Blackhole::FinalTargetsReceived);
	BlackholeTargetingTask->Cancelled.AddDynamic(this,&UGA_Blackhole::FinalTargetsReceived);
	BlackholeTargetingTask->ReadyForActivation();
	
	AGameplayAbilityTargetActor* TargetActor;
	BlackholeTargetingTask->BeginSpawningActor(this,BlackholeTargetActorClass,TargetActor);
	ATA_Blackhole* TargetActor_Blackhole = Cast<ATA_Blackhole>(TargetActor);
	if (TargetActor_Blackhole)
	{
		TargetActor_Blackhole->ConfigureBlackhole(TargetAreaRadius,BlackholePullSpeed,BlackholeDuration,GetOwnerTeamId());
		
	}
	BlackholeTargetingTask->FinishSpawningActor(this,TargetActor);
	if (TargetActor_Blackhole)
	{
		TargetActor_Blackhole->SetActorLocation(UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle,1).ImpactPoint);
	}	
}

void UGA_Blackhole::PlacementCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}

void UGA_Blackhole::FinalTargetsReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	FVector BlowCenter = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle,1).ImpactPoint;
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(TargetDataHandle,FinalBlowDamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		PushTargetsFromLocation(TargetDataHandle,BlowCenter,BlowPushSpeed);


		UAbilityTask_PlayMontageAndWait* FinalBlowMontageTask =UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,FinalBlowMontage); 

		FinalBlowMontageTask->OnBlendOut.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
		FinalBlowMontageTask->OnCancelled.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
		FinalBlowMontageTask->OnCompleted.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);
		FinalBlowMontageTask->OnInterrupted.AddDynamic(this,&UGA_Blackhole::K2_EndAbility);

		FinalBlowMontageTask->ReadyForActivation();
	}else
	{
		PlayMontageLocally(FinalBlowMontage);
	}

	FGameplayCueParameters FinalBlowCueParameters;
	FinalBlowCueParameters.Location = BlowCenter;
	FinalBlowCueParameters.RawMagnitude = TargetAreaRadius;

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(FinalBlowCueTag,FinalBlowCueParameters);
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(UCAbilitySystemStatics::GetCameraShakeGameplayCueTag(),FinalBlowCueParameters);
}

void UGA_Blackhole::AddAimEffect()
{
	AimEffectHandle = BP_ApplyGameplayEffectToOwner(AimEffect);
}

void UGA_Blackhole::RemoveAimEffect()
{
	if (AimEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(AimEffectHandle); 
	}
}

void UGA_Blackhole::AddFocusEffect()
{
	FocusEffectHandle = BP_ApplyGameplayEffectToOwner(FocusEffect);
}

void UGA_Blackhole::RemoveFocusEffect()
{
	if (FocusEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(FocusEffectHandle);
	}
}
