#include "CGameplayAbility.h"

#include "Components/SkeletalMeshComponent.h"

UAnimInstance* UCGameplayAbility::GetOwnerAnimInstance() const
{
	// Codex: Lesson 38 - Resolve the animation instance from the actor info
	// instead of reaching into a concrete character class.
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	return OwnerSkeletalMeshComp ? OwnerSkeletalMeshComp->GetAnimInstance() : nullptr;
}
