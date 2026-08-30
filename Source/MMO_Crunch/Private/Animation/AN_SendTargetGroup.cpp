#include "AN_SendTargetGroup.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayCueManager.h"
#include "Kismet/KismetSystemLibrary.h"
void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp)return;

	if (TargetSocketNames.Num() <=1) return;

	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner())) return;

	FGameplayEventData EventData;
	TSet<AActor*> HitActors;
	AActor* OwnActor = MeshComp->GetOwner();
	const IGenericTeamAgentInterface* OwnerInterface = Cast<IGenericTeamAgentInterface>(OwnActor);
	for (int i = 1; i < TargetSocketNames.Num(); ++i)
	{
		FVector StartLocation = MeshComp->GetSocketLocation(TargetSocketNames[i-1]);
		FVector EndLocation = MeshComp->GetSocketLocation(TargetSocketNames[i]);

		TArray<FHitResult> HitResults;
		
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<AActor*>ActorsToIgnore; 
		if (bIgnoreSelf)
		{
			ActorsToIgnore.Add(OwnActor);
		}

		EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		UKismetSystemLibrary::SphereTraceMultiForObjects(MeshComp,StartLocation,EndLocation,
			SphereSweepRadius,ObjectTypes,false,ActorsToIgnore,DrawDebugTrace,HitResults ,false
			);
		for (const FHitResult& HitResult : HitResults)
		{
			if (HitActors.Contains(HitResult.GetActor()))
			{
				continue;
			}
			HitActors.Add(HitResult.GetActor());
			if (OwnerInterface)
			{
				if (OwnerInterface->GetTeamAttitudeTowards(*HitResult.GetActor()) != TargetTeam)
				{
					continue;
				}
			}
			FGameplayAbilityTargetData_SingleTargetHit* HitTarget = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
			EventData.TargetData.Add(HitTarget);
			SendLocalGameplayCue(HitResult);
		}
	}
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(),EventTag,EventData);
}

void UAN_SendTargetGroup::SendLocalGameplayCue(const FHitResult& HitResult) const
{
	FGameplayCueParameters Parameters;
	Parameters.Location = HitResult.Location;
	Parameters.Normal = HitResult.ImpactPoint;

	for (const FGameplayTag& GameplayTag: TriggerGameplayCueTag)
	{
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(HitResult.GetActor(),GameplayTag,EGameplayCueEvent::Executed,Parameters);
	}
}
