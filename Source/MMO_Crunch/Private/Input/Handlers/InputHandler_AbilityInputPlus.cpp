// Fill out your copyright notice in the Description page of Project Settings.
#include "InputHandler_AbilityInputPlus.h"

#include "AbilitySystemComponent.h"
#include "MapleInputHandlerHelpers.h"


UInputHandler_AbilityInputPlus::UInputHandler_AbilityInputPlus()
{
	InputID = ECAbilityInputId::None;
}

bool UInputHandler_AbilityInputPlus::CanHandle_Implementation(const ETriggerEvent& TriggerEvent,
	const UInputAction* InputAction) const
{
	const bool bHasClass = ensureMsgf(InputID != ECAbilityInputId::None, TEXT("Please ensure the Input ID is valid!"));
	return Super::CanHandle_Implementation(TriggerEvent, InputAction) && bHasClass;
}

UGameplayAbility* UInputHandler_AbilityInputPlus::GetAbilityInstance(const UMapleInputManagerComponent* Manager) const
{
	const TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = Manager->GetAbilitySystemComponent();
    
	if (ensureMsgf(IsValid(AbilitySystemComponent), TEXT("No ASC received from the Input Manager.")) &&
		ensureMsgf(InputID != ECAbilityInputId::None, TEXT("The Input ID must be equal or greater than zero.")))
	{
		TArray<const FGameplayAbilitySpec*> Specs;
		AbilitySystemComponent->FindAllAbilitySpecsFromInputID((int32)InputID, Specs);

		for (const FGameplayAbilitySpec* Spec : Specs)
		{
			if (Spec != nullptr && Spec->Handle.IsValid())
			{
				UGameplayAbility* Ability = Spec->GetPrimaryInstance(); 
				if (Ability == nullptr)
				{
					Ability = Spec->Ability;
				}

				return Ability;
			}
		}
	}

	return nullptr;
}

bool UInputHandler_AbilityInputPlus::ActivateAbility(UMapleInputManagerComponent* Manager,
	const FInputActionValue& Value, const UInputAction* InputAction) const
{
	 UAbilitySystemComponent* AbilitySystemComponent = Manager->GetAbilitySystemComponent();
        if (ensure(IsValid(AbilitySystemComponent)))
        {
            UE_LOG(LogMapleInputHandler, Verbose, TEXT("[%s] Action %s is triggering an ability with ID %d."),
                    *GetNameSafe(Manager->GetOwner()), *GetNameSafe(InputAction), InputID);
    
            AbilitySystemComponent->AbilityLocalInputPressed((int32)InputID);
        	return true;
        }
    
    	return false;
}

bool UInputHandler_AbilityInputPlus::CancelAbility(UMapleInputManagerComponent* Manager, const FInputActionValue& Value,
	const UInputAction* InputAction) const
{
	return FMapleInputHandlerHelpers::InterruptAbilityByInputID(Manager, InputAction, (int32)InputID);
}
