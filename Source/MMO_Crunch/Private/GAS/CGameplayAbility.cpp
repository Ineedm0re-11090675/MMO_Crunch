#include "CGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/MapleCombatAttributeSet.h"
#include "Character/CCharacter.h"
#include "CAbilitySystemStatics.h"
#include "GAP_Launched.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
// Codex: Apply the migrated damage contract to MapleCombat targets.
#include "MapleCombatTags.h"

UCGameplayAbility::UCGameplayAbility()
{
	ActivationBlockedTags.AddTag(UCAbilitySystemStatics::GetStunStatsAbilityTag()); 
}

bool UCGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (AbilitySpec && AbilitySpec->Level <=0 )
	{
		return false;
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags); 
}

AActor* UCGameplayAbility::GetAimTarget(float AimDistance, ETeamAttitude::Type TeamAttitude) const
{
	AActor* OwnAvatarActor = GetAvatarActorFromActorInfo();
	if (OwnAvatarActor)
	{
		FVector Location;
		FRotator Rotation;
		OwnAvatarActor->GetActorEyesViewPoint(Location, Rotation);
		FVector AimEndLoc = Location + Rotation.Vector() * AimDistance;
		TArray<FHitResult> OutHitResults;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(OwnAvatarActor);

		FCollisionObjectQueryParams CollisionObjectParams;
		CollisionObjectParams.AddObjectTypesToQuery(ECC_Pawn);

		if (ShouldDrawDebugSphere())
		{
			DrawDebugLine(GetWorld(),Location,AimEndLoc,FColor::Red,false,2.f,0U,3.f);
		}
		
		if (GetWorld()->LineTraceMultiByObjectType(OutHitResults,Location,AimEndLoc,CollisionObjectParams,CollisionQueryParams))
		{
			for (const FHitResult& Hit : OutHitResults)
			{
				if (IsActorTeamAttitudeIs(Hit.GetActor(),TeamAttitude))
				{
					return Hit.GetActor();
				}
			}
		}
	}
	return nullptr; 
}


UAnimInstance* UCGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* MeshComponent =GetOwningComponentFromActorInfo();
	if (MeshComponent)
	{
		return MeshComponent->GetAnimInstance();
	}
	return nullptr; 
}

TArray<FHitResult> UCGameplayAbility::GetHitResultsFromSweepLocationTargetData (
	FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereRadius,ETeamAttitude::Type TargetTeam, bool bShowDebugSphere, bool bIgnoreSelf) const
{
	TArray<FHitResult>OutHitResults;
	TArray<AActor*>ActorsGetHit;

	IGenericTeamAgentInterface* GenericTeamAgentInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	
	
	for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
	{
		FVector StartLoc =TargetData->GetOrigin().GetTranslation();
		FVector EndLoc = TargetData->GetEndPoint();

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<AActor*>ActorToIgnore;
		if (bIgnoreSelf)
		{
			ActorToIgnore.Add(GetAvatarActorFromActorInfo());
		}
		EDrawDebugTrace::Type DebugTraceType =bShowDebugSphere ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		TArray<FHitResult> HitResults;
		UKismetSystemLibrary::SphereTraceMultiForObjects(this,StartLoc,EndLoc,SphereRadius,ObjectTypes,false,ActorToIgnore,DebugTraceType,HitResults,false);
		//LineTrace VS SphereTrace --->线 vs 香肠

		for (FHitResult& Hit : HitResults)
		{
			if (ActorsGetHit.Contains(Hit.GetActor()))
			{
				continue;
			}
			if (GenericTeamAgentInterface)
			{
				ETeamAttitude::Type OtherActorTeamAttitude = GenericTeamAgentInterface->GetTeamAttitudeTowards(*Hit.GetActor());
				if (OtherActorTeamAttitude != TargetTeam)
				{
					continue;
				}
			}
			ActorsGetHit.Add(Hit.GetActor());
			OutHitResults.Add(Hit);
		}
	}
	return OutHitResults;
}

void UCGameplayAbility::PushSelf(const FVector& PushForce)
{
	ACharacter* OwnCharacter = GetAvatarCharacter();
	if (OwnCharacter)
	{
		OwnCharacter->LaunchCharacter(PushForce,true,true);
	}
}

void UCGameplayAbility::PushTarget(AActor* Target, const FVector& PushForce)
{
	if (!Target) return;

	FGameplayEventData EventData;

	//储存数据，在Uppercut储存，在 Launched里读取
	FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	FHitResult HitResult;
	HitResult.ImpactNormal = PushForce;
	TargetData->HitResult = HitResult;
	EventData.TargetData.Add(TargetData);

	// Codex: ACCharacter's generic AbilitySystemInterface still returns the legacy ASC; send launch events to Maple ASC explicitly.
	UAbilitySystemComponent* TargetASC = nullptr;
	if (const ACCharacter* CrunchTarget = Cast<ACCharacter>(Target))
	{
		TargetASC = CrunchTarget->GetMapleAbilitySystemComponent();
	}
	else
	{
		TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	}

	if (IsValid(TargetASC))
	{
		TargetASC->HandleGameplayEvent(UGAP_Launched::GetLaunchedAbilityActivationTag(), &EventData);
	}
}

void UCGameplayAbility::PushTargets(const TArray<AActor*> Actors, const FVector& PushForce)
{
	for (AActor* Actor : Actors)
	{
		PushTarget(Actor,PushForce);
	}
}

void UCGameplayAbility::PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushForce)
{
	TArray<AActor*> Targets = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	PushTargets(Targets,PushForce);
}

void UCGameplayAbility::PushTargetsFromLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	const FVector& FromLocation, float PullSpeed)
{
	TArray<AActor*> Targets = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	PushTargetsFromLocation(Targets,FromLocation,PullSpeed);
}

void UCGameplayAbility::PushTargetsFromLocation(const TArray<AActor*>& Targets, const FVector& FromLocation,
	float PushSpeed)
{
	for(AActor* Target : Targets)
	{
		FVector PushDir = Target->GetActorLocation() - FromLocation;
		PushDir.Z = 0.f;
		PushDir.Normalize();

		PushTarget(Target,PushDir * PushSpeed);
	}
}

void UCGameplayAbility::PushTargetsFromOwnerLocation(const TArray<AActor*>& Targets, float PushSpeed)
{
	AActor* AvatarActor =GetAvatarActorFromActorInfo();
	if (AvatarActor)
	{
		FVector OwnerLocation = AvatarActor->GetActorLocation();
		PushTargetsFromLocation(Targets,OwnerLocation,PushSpeed);
	}
}

void UCGameplayAbility::PushTargetsFromOwnerLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	float PushSpeed)
{
	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	PushTargetsFromOwnerLocation(TargetActors, PushSpeed);
}

void UCGameplayAbility::PlayMontageLocally(UAnimMontage* MontageToPlay)
{
	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (OwnerAnimInst && !OwnerAnimInst->Montage_IsPlaying(MontageToPlay))
	{
		OwnerAnimInst->Montage_Play(MontageToPlay);
	}
}

void UCGameplayAbility::StopMontageAfterCurrentSection(UAnimMontage* MontageToStop)
{
	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (OwnerAnimInst)
	{
		FName CurrentSectionName = OwnerAnimInst->Montage_GetCurrentSection(MontageToStop);
		OwnerAnimInst->Montage_SetNextSection(CurrentSectionName,NAME_None,MontageToStop);
	}
}


ACharacter* UCGameplayAbility::GetAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());	
	}
	return AvatarCharacter;
}

// Codex: Use Maple ASC as the damage source while Crunch abilities are still granted by the legacy ASC.
UAbilitySystemComponent* UCGameplayAbility::GetMapleCombatSourceASC() const
{
	if (const ACCharacter* CrunchCharacter = Cast<ACCharacter>(GetAvatarActorFromActorInfo()))
	{
		return CrunchCharacter->GetMapleAbilitySystemComponent();
	}

	return GetAbilitySystemComponentFromActorInfo();
}

void UCGameplayAbility::ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult,
	TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect,Level);

	//将HitResult加入到GE的context信息内部，让BluePrint能读到HitResult，然后让GC读到FVX方向
	FGameplayEffectContextHandle ContextHandle =MakeEffectContext(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo());
	ContextHandle.AddHitResult(HitResult);
	EffectSpecHandle.Data->SetContext(ContextHandle);
		
	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(),CurrentActorInfo,
		CurrentActivationInfo,
		EffectSpecHandle,
		UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor())
		);
		
}

// Codex: Apply one MapleCombat damage spec to each actor in target data, resolving Crunch characters to their Maple ASC.
void UCGameplayAbility::ApplyMapleCombatDamageToTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	TSubclassOf<UGameplayEffect> DamageEffect,
	const float DamageAmount,
	const int32 Level)
{
	if (!DamageEffect || DamageAmount <= 0.0f)
	{
		return;
	}

	for (int32 TargetDataIndex = 0; TargetDataIndex < TargetDataHandle.Num(); ++TargetDataIndex)
	{
		const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(TargetDataIndex);
		const FHitResult* HitResult = TargetData ? TargetData->GetHitResult() : nullptr;
		const TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(
			TargetDataHandle, TargetDataIndex);
		for (AActor* TargetActor : TargetActors)
		{
			const FHitResult* TargetHitResult = HitResult && HitResult->GetActor() == TargetActor ? HitResult : nullptr;
			ApplyMapleCombatDamageToActor(TargetActor, DamageEffect, DamageAmount, Level, TargetHitResult);
		}
	}
}

// Codex: Keep the originating hit in the effect context for MapleCombat damage outcomes and cosmetics.
void UCGameplayAbility::ApplyMapleCombatDamageToHitResultActor(
	const FHitResult& HitResult,
	TSubclassOf<UGameplayEffect> DamageEffect,
	const float DamageAmount,
	const int32 Level)
{
	ApplyMapleCombatDamageToActor(HitResult.GetActor(), DamageEffect, DamageAmount, Level, &HitResult);
}

// Codex: Use the target's Maple ASC for Crunch characters whose legacy interface still returns the old ASC.
void UCGameplayAbility::ApplyMapleCombatDamageToActor(
	AActor* TargetActor,
	TSubclassOf<UGameplayEffect> DamageEffect,
	const float DamageAmount,
	const int32 Level,
	const FHitResult* ContextHitResult)
{
	UAbilitySystemComponent* SourceASC = GetMapleCombatSourceASC();
	// Codex: MapleCombat damage is server-authoritative even when an ability also predicts local hit reactions.
	if (!K2_HasAuthority() || !IsValid(SourceASC) || !IsValid(TargetActor) || !DamageEffect || DamageAmount <= 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = nullptr;
	if (ACCharacter* CrunchCharacter = Cast<ACCharacter>(TargetActor))
	{
		TargetASC = CrunchCharacter->GetMapleAbilitySystemComponent();
	}
	else
	{
		TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	}

	if (!IsValid(TargetASC) || !TargetASC->GetSet<UMapleCombatAttributeSet>())
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	if (ContextHitResult)
	{
		EffectContext.AddHitResult(*ContextHitResult, true);
	}

	FGameplayEffectSpecHandle DamageSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, Level, EffectContext);
	if (!DamageSpecHandle.IsValid())
	{
		return;
	}

	DamageSpecHandle.Data->SetSetByCallerMagnitude(Tag_Combat_Data_Damage, DamageAmount);
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}

bool UCGameplayAbility::IsActorTeamAttitudeIs(const AActor* OtherActor, ETeamAttitude::Type TargetTeam) const
{
	if (!OtherActor) return false;
	IGenericTeamAgentInterface* GenericTeamAgentInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	if (GenericTeamAgentInterface)
	{
		return GenericTeamAgentInterface->GetTeamAttitudeTowards(*OtherActor) == TargetTeam;
	}
	return false;
}

void UCGameplayAbility::SendLocalGameplayEvent(const FGameplayTag& Tag, const FGameplayEventData& Data)
{
	UAbilitySystemComponent* OwnASC = GetAbilitySystemComponentFromActorInfo();
	if (OwnASC)
	{
		OwnASC->HandleGameplayEvent(Tag,&Data);
	}
}
