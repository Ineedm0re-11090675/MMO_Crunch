#include "Animation/AN_SendGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

void UAN_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner() || !EventTag.IsValid())
	{
		return;
	}

	// Codex: Lesson 36 - Resolve the ASC generically so the notify works on any
	// actor that exposes an AbilitySystemComponent, not only the player class.
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!OwnerASC)
	{
		return;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, FGameplayEventData());
}

FString UAN_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (!EventTag.IsValid())
	{
		return TEXT("None");
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	return TagNames.Num() > 0 ? TagNames.Last().ToString() : TEXT("None");
}
