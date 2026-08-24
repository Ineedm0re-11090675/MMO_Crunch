#include "AN_SendGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
void UAN_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp->GetOwner()) return;
	UAbilitySystemComponent * ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!ASC) return;
	//此Tag非GA的Tag，是EventTag，标志着发生了什么，三个入参分别是 ： 打给谁的GAS，发生了什么，附带什么信息
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(),EventTag,FGameplayEventData());
}

FString UAN_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		TArray<FName> TagName;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag,TagName);
		return TagName.Last().ToString();
	}
	return "None"; 
}
