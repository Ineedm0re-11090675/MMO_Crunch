#include "AN_SendTargetGroup.h"

#include "AbilitySystemBlueprintLibrary.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp)return;

	if (TargetSocketNames.Num() <=1) return;

	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner())) return;

	FGameplayEventData EventData;
	for (int i = 1; i < TargetSocketNames.Num(); ++i)
	{
		FGameplayAbilityTargetData_LocationInfo* TargetLocationInfo = new FGameplayAbilityTargetData_LocationInfo();
		FVector StartLocation = MeshComp->GetSocketLocation(TargetSocketNames[i-1]);
		FVector EndLocation = MeshComp->GetSocketLocation(TargetSocketNames[i]);
		TargetLocationInfo->SourceLocation.LiteralTransform.SetLocation(StartLocation);
		TargetLocationInfo->TargetLocation.LiteralTransform.SetLocation(EndLocation); 

		EventData.TargetData.Add(TargetLocationInfo);
	}
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(),EventTag,EventData);
}
