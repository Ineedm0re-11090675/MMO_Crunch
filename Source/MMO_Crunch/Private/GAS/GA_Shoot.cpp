#include "GA_Shoot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAbilitySystemStatics.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/ProjectileActor.h"
UGA_Shoot::UGA_Shoot()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimingStatsAbilityTag());
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetCrossHairTag());
}

void UGA_Shoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitStartShootEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetBasicAttackPressedTag());
		WaitStartShootEvent->EventReceived.AddDynamic(this,&UGA_Shoot::StartShooting);
		WaitStartShootEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitStopShootEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetBasicAttackReleasedTag());
		WaitStopShootEvent->EventReceived.AddDynamic(this,&UGA_Shoot::StopShooting);
		WaitStopShootEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootProjectileEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetShootTag(),nullptr,false,false);
		WaitShootProjectileEvent->EventReceived.AddDynamic(this,&UGA_Shoot::ShootProjectile);
		WaitShootProjectileEvent->ReadyForActivation();
	}
}

void UGA_Shoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	
	K2_EndAbility();
}

void UGA_Shoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AimTargetASC)
	{
		AimTargetASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeathStatsAbilityTag()).RemoveAll(this);
		AimTargetASC = nullptr;
	}
	StopShooting(FGameplayEventData());
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);	
}

FGameplayTag UGA_Shoot::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.shoot");
}

void UGA_Shoot::StartShooting(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayShootMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ShootMontage);
		PlayShootMontage->ReadyForActivation();
	}else
	{
		PlayMontageLocally(ShootMontage);
	}
	/*
	UAbilityTask_NetworkSyncPoint* NetworkSyncPoint = UAbilityTask_NetworkSyncPoint::WaitNetSync(this,EAbilityTaskNetSyncType::OnlyServerWait);
	NetworkSyncPoint->ReadyForActivation();
	
	UAbilityTask_PlayMontageAndWait* PlayShootMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ShootMontage);
	PlayShootMontage->ReadyForActivation();
	*/
	FindAimTarget();
	StartAimTargetCheckTimer();
}

void UGA_Shoot::StopShooting(FGameplayEventData Payload)
{
	if (ShootMontage)
	{
		StopMontageAfterCurrentSection(ShootMontage);
	}
	StopAimTargetCheckTimer();
}

void UGA_Shoot::ShootProjectile(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerAvatarActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		FVector SocketLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
		USkeletalMeshComponent* MeshComp =GetOwningComponentFromActorInfo();
		if (MeshComp)
		{
			TArray<FName> OutNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(Payload.EventTag,OutNames);
			if (OutNames.Num() != 0 )
			{
				FName SocketName = OutNames.Last();
				SocketLocation = MeshComp->GetSocketLocation(SocketName);
			}
		}
		AProjectileActor* Projectile = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass,SocketLocation,OwnerAvatarActor->GetActorRotation(),SpawnParams);

		if (Projectile)
		{
			Projectile->ShootProjectile(
				ShootProjectileSpeed,
				ShootProjectileRange,
				GetAimTargetIfValid(),
				GetOwnerTeamId(),
				MakeOutgoingGameplayEffectSpec(
					ProjectileHitEffect,
					GetAbilityLevel(
						CurrentSpecHandle,
						CurrentActorInfo)));
			
		}
	}
}

AActor* UGA_Shoot::GetAimTargetIfValid() const
{
	if (HasValidTarget())
		return AimTarget;
	return nullptr;
}

void UGA_Shoot::FindAimTarget()
{
	if (!HasValidTarget()) return;

	if (AimTargetASC)
	{
		AimTargetASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeathStatsAbilityTag()).RemoveAll(this);
		AimTargetASC = nullptr;
	}
	AimTarget = GetAimTarget(ShootProjectileRange,ETeamAttitude::Hostile);
	if (AimTarget)
	{
		AimTargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AimTarget);
		if (AimTargetASC)
		{
			AimTargetASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeathStatsAbilityTag()).AddUObject(this,&UGA_Shoot::TargetDeadTagUpdate);
		}
	}
}

void UGA_Shoot::StartAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(AimTargetCheckTimerHandle,this,&UGA_Shoot::FindAimTarget,AimTargetCheckTimeInterval,true);
	}
}

void UGA_Shoot::StopAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AimTargetCheckTimerHandle);
	}
}

bool UGA_Shoot::HasValidTarget() const
{
	if (!AimTarget) return false;
	if (UCAbilitySystemStatics::IsActorDead(AimTarget)) return false;
	if (!IsActorInRange()) return false;
	return true;
	
}

bool UGA_Shoot::IsActorInRange() const
{
	if (!AimTarget)
		return false;
	float Dist =FVector::Distance(AimTarget->GetActorLocation(),GetAvatarActorFromActorInfo()->GetActorLocation());
	return Dist <= ShootProjectileRange;
}

void UGA_Shoot::TargetDeadTagUpdate(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0) FindAimTarget(); 
}
