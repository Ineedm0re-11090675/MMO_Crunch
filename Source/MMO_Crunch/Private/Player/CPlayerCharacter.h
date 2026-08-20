#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "../Character/CCharacter.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "CPlayerCharacter.generated.h"

UCLASS()
class ACPlayerCharacter : public ACCharacter 
{
	GENERATED_BODY()
public:
	ACPlayerCharacter();
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* CameraComp;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GamePlayInputMapContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	// Codex: Lesson 35 - Associates each Enhanced Input action with the
	// enum value stored on its gameplay ability spec.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<ECAbilityInputID, class UInputAction*> GameplayAbilityInputActions;
	
	void HandleLook(const FInputActionValue& LookActionValue);
	void HandleMove(const FInputActionValue& MoveActionValue);

	// Codex: Lesson 35 - Forwards local input state into the ASC.
	void HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID);

	FVector GetMoveForwardVector() const;
	FVector GetLookRightVector() const;
	FVector GeLookForwardVector() const;
};
