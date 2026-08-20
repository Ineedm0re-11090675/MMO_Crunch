#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbilityTypes.generated.h"

// Codex: Lessons 34-35 - Gives each gameplay ability a stable input ID so the
// player character and the ability system can communicate without hard-coded ints.
UENUM(BlueprintType)
enum class ECAbilityInputID : uint8
{
	None UMETA(DisplayName = "None"),
	BasicAttack UMETA(DisplayName = "Basic Attack"),
	AbilityOne UMETA(DisplayName = "Ability One"),
	AbilityTwo UMETA(DisplayName = "Ability Two"),
	AbilityThree UMETA(DisplayName = "Ability Three"),
	AbilityFour UMETA(DisplayName = "Ability Four"),
	AbilityFive UMETA(DisplayName = "Ability Five"),
	AbilitySix UMETA(DisplayName = "Ability Six"),
	Confirm UMETA(DisplayName = "Confirm"),
	Cancel UMETA(DisplayName = "Cancel")
};
