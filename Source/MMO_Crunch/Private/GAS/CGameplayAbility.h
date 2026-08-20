#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CGameplayAbility.generated.h"

class UAnimInstance;

UCLASS()
class UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	// Codex: Lesson 38 - Provides combo abilities with the owning mesh's
	// animation instance so they can change the montage's next section.
	UAnimInstance* GetOwnerAnimInstance() const;
};
