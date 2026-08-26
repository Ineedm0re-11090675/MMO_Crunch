#include "CGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CAbilitySystemStatics.h"
#include "GAP_Launched.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UCGameplayAbility::UCGameplayAbility()
{
	ActivationBlockedTags.AddTag(UCAbilitySystemStatics::GetStunStatsAbilityTag()); 
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
	EventData.TargetData .Add(TargetData);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target,UGAP_Launched::GetLaunchedAbilityActivationTag(),EventData);
}

ACharacter* UCGameplayAbility::GetAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());	
	}
	return AvatarCharacter;
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
