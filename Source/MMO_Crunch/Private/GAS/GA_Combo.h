#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_Combo.generated.h"

UCLASS()
class UGA_Combo : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Combo();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// Codex: Lesson 37 - The parent tag lets the ability receive all combo
	// section events, while the end tag is handled separately below.
	static FGameplayTag GetComboChangedEventTag();
	static FGameplayTag GetComboChangedEventEndTag();

private:
	// Codex: Lesson 38 - Keep listening for the next input while the montage
	// is playing, then commit the requested montage section transition.
	void SetupWaitComboInputPress();

	UFUNCTION()
	void HandleInputPress(float TimeWaited);

	void TryCommitCombo();

	UFUNCTION()
	void ComboChangedEventReceived(FGameplayEventData Data);

	UPROPERTY(EditDefaultsOnly,Category = "Animation")
	UAnimMontage* ComboMontage;

	FName NextComboName;
};
