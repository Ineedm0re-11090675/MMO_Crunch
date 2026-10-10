// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "Input/Handlers/InputHandler_AbilityActivation.h"
#include "InputHandler_AbilityInputPlus.generated.h"

class UMapleInputManagerComponent;
/**
 * 
 */
UCLASS(DisplayName = "GAS: Activate Ability by Input Enum")
class MMO_CRUNCH_API UInputHandler_AbilityInputPlus : public UInputHandler_AbilityActivation
{
	GENERATED_BODY()
	
public:
	UInputHandler_AbilityInputPlus();
	virtual bool CanHandle_Implementation(const ETriggerEvent& TriggerEvent, const UInputAction* InputAction) const override;

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Activation")
	ECAbilityInputId InputID;

	
	// -- Begin Ability Activation Handler implementation
	virtual UGameplayAbility* GetAbilityInstance(const UMapleInputManagerComponent* Manager) const override;
	virtual bool ActivateAbility(UMapleInputManagerComponent* Manager, const FInputActionValue& Value, const UInputAction* InputAction) const final override;
	virtual bool CancelAbility(UMapleInputManagerComponent* Manager, const FInputActionValue& Value, const UInputAction* InputAction) const final override;
	// -- End Ability Activation Handler implementation
};
