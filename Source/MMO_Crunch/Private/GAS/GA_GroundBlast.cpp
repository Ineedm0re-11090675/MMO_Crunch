#include "GA_GroundBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimInstance.h"
#include "CAbilitySystemStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAP_Launched.h"
#include "MapleCombatTags.h"
#include "TargetActor_GroundPick.h"
#include "Character/CCharacter.h"

// Codex: Use MapleCombat damage execution and confirmation targeting while preserving the old Crunch ability tags.
#include "AbilitySystem/Effects/CombatEffect_Damage.h"

UGA_GroundBlast::UGA_GroundBlast(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Codex: Match the old flow: start the aiming animation immediately, wait for player confirmation, then commit cost/cooldown.
	CastTrigger = ECombatCastTrigger::AbilityActivation;
	Targeting = ECombatCastTargeting::WaitForConfirmation;
	CommitTime = ECombatCastCommitTime::TargetsAcquired;
	ConfirmationType = EGameplayTargetingConfirmation::UserConfirmed;
	CastEffectClass = UCombatEffect_Damage::StaticClass();
	// Codex: Keep the interactive cast alive after the short aim montage ends so the player can still confirm the target.
	bMatchAnimationEnd = false;

	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimingStatsAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_GroundBlast::ModifyActor_Implementation(AGameplayAbilityTargetActor* TargetActor)
{
	Super::ModifyActor_Implementation(TargetActor);

	ATargetActor_GroundPick* GroundPickActor = Cast<ATargetActor_GroundPick>(TargetActor);
	if (!IsValid(GroundPickActor))
	{
		return;
	}

	// Codex: Preserve GroundBlast's Blueprint-configured placement radius, trace range, and debug visualization.
	GroundPickActor->SetShouldDrawDebug(bShowDebugSphere);
	GroundPickActor->SetTargetAreaRadius(TargetAreaRadius);
	GroundPickActor->SetTargetTraceRange(TargetTraceRange);
}

TSubclassOf<AGameplayAbilityTargetActor> UGA_GroundBlast::GetTargetingActorClass_Implementation() const
{
	// Codex: Return the legacy property so existing GroundBlast blueprint subclasses need no field migration in the editor.
	return TargetActorClass;
}

UAnimMontage* UGA_GroundBlast::GetAnimationMontage_Implementation(FGameplayTagContainer AbilityTags) const
{
	// Codex: Maple's default Combat Animation Provider asks this interface before its own default montage.
	static_cast<void>(AbilityTags);
	return GroundBlastMontage;
}

float UGA_GroundBlast::GetCastEffectLevel_Implementation() const
{
	// Codex: Keep the prior behavior of creating the damage spec at this ability's granted level.
	return static_cast<float>(GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
}

void UGA_GroundBlast::ApplyCastMagnitudesToSpec(const FGameplayEffectSpecHandle& EffectSpecHandle,
	const int32 TargetCount) const
{
	Super::ApplyCastMagnitudesToSpec(EffectSpecHandle, TargetCount);

	if (!EffectSpecHandle.IsValid())
	{
		return;
	}

	// Codex: Send the positive configured damage through MapleCombat's SetByCaller contract.
	EffectSpecHandle.Data->SetSetByCallerMagnitude(Tag_Combat_Data_Damage, GroundBlastDamageAmount);
}

void UGA_GroundBlast::ApplyEffectToTargets(FGameplayAbilityTargetDataHandle TargetDataHandle,
	FGameplayEffectSpecHandle EffectSpecHandle)
{
	if (TargetDataHandle.Num() == 0 || !EffectSpecHandle.IsValid())
	{
		return;
	}

	// Codex: Mirror MapleCombat's target-acquired commit gate so Crunch push/cues only run after commit succeeds.
	if (CommitTime == ECombatCastCommitTime::TargetsAcquired && !K2_CommitAbility())
	{
		return;
	}

	ApplyCastMagnitudesToSpec(EffectSpecHandle, TargetDataHandle.Num());

	if (K2_HasAuthority())
	{
		// Codex: The generic GAS TargetData helper resolves CCharacter through its legacy ASC interface, so apply to Maple ASC explicitly.
		if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
		{
			for (const TSharedPtr<FGameplayAbilityTargetData>& TargetData : TargetDataHandle.Data)
			{
				if (!TargetData.IsValid())
				{
					continue;
				}

				for (const TWeakObjectPtr<AActor>& TargetActor : TargetData->GetActors())
				{
					if (!IsValid(TargetActor.Get()))
					{
						continue;
					}

					UAbilitySystemComponent* TargetASC = nullptr;
					if (const ACCharacter* CrunchTarget = Cast<ACCharacter>(TargetActor.Get()))
					{
						TargetASC = CrunchTarget->GetMapleAbilitySystemComponent();
					}
					else
					{
						TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor.Get());
					}

					if (!IsValid(TargetASC))
					{
						continue;
					}

					FGameplayEffectSpec TargetSpec(*EffectSpecHandle.Data.Get());
					FGameplayEffectContextHandle TargetContext = TargetSpec.GetContext().Duplicate();
					TargetSpec.SetContext(TargetContext);
					TargetData->AddTargetDataToContext(TargetContext, false);
					SourceASC->ApplyGameplayEffectSpecToTarget(TargetSpec, TargetASC,
						SourceASC->GetPredictionKeyForNewAction());
				}
			}
		}

		PushTargets(TargetDataHandle, GroundBlastPushVelocity);
	}

	// Codex: The target actor appends the placement hit last; this also handles an empty actor list safely.
	const int32 PlacementDataIndex = TargetDataHandle.Num() - 1;
	const FHitResult PlacementHit = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(
		TargetDataHandle, PlacementDataIndex);

	FGameplayCueParameters CueParameters;
	CueParameters.Location = PlacementHit.ImpactPoint;
	CueParameters.RawMagnitude = TargetAreaRadius;

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->ExecuteGameplayCue(BlastGameplayCueTag, CueParameters);
		AbilitySystemComponent->ExecuteGameplayCue(UCAbilitySystemStatics::GetCameraShakeGameplayCueTag(), CueParameters);
	}

	// Codex: Keep the existing post-confirm GroundBlast cast flourish while Maple owns the main ability montage.
	if (IsValid(CastMontage))
	{
		if (USkeletalMeshComponent* OwningMesh = GetOwningComponentFromActorInfo())
		{
			if (UAnimInstance* AnimInstance = OwningMesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(CastMontage);
			}
		}
	}
}

void UGA_GroundBlast::PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	const FVector& PushVelocity)
{
	// Codex: Route the launch trigger to the Maple ASC that receives GAP_Launched from DA_CrunchGAS.
	const FGameplayTag LaunchTag = UGAP_Launched::GetLaunchedAbilityActivationTag();
	const TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);

	for (AActor* TargetActor : TargetActors)
	{
		if (!IsValid(TargetActor))
		{
			continue;
		}

		FGameplayEventData EventData;
		EventData.Instigator = GetAvatarActorFromActorInfo();
		EventData.Target = TargetActor;

		FGameplayAbilityTargetData_SingleTargetHit* LaunchData = new FGameplayAbilityTargetData_SingleTargetHit();
		LaunchData->HitResult.ImpactNormal = PushVelocity;
		EventData.TargetData.Add(LaunchData);

		UAbilitySystemComponent* TargetAbilitySystemComponent = nullptr;
		if (const ACCharacter* CrunchTarget = Cast<ACCharacter>(TargetActor))
		{
			TargetAbilitySystemComponent = CrunchTarget->GetMapleAbilitySystemComponent();
		}
		else
		{
			TargetAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
		}

		if (IsValid(TargetAbilitySystemComponent))
		{
			TargetAbilitySystemComponent->HandleGameplayEvent(LaunchTag, &EventData);
		}
	}
}
