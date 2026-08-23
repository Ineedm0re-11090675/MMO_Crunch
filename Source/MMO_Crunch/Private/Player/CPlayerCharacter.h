#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "../Character/CCharacter.h"
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
	
	void HandleLook(const FInputActionValue& LookActionValue);
	void HandleMove(const FInputActionValue& MoveActionValue);

	FVector GetMoveForwardVector() const;
	FVector GetLookRightVector() const;
	FVector GeLookForwardVector() const;
};
