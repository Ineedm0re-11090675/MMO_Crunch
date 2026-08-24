#include "CGameplayAbility.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"

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
