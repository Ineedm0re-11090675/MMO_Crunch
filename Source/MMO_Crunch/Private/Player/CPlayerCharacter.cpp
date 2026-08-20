#include "CPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
ACPlayerCharacter::ACPlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Camera Spring");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComp->SetupAttachment(SpringArm,USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;

	// Rotate With Controller viewDir
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
}

void ACPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* OwningController = GetController<APlayerController>();
	if (OwningController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputLocalPlayerSubsystem = OwningController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputLocalPlayerSubsystem)
		{
			InputLocalPlayerSubsystem->RemoveMappingContext(GamePlayInputMapContext);
			InputLocalPlayerSubsystem->AddMappingContext(GamePlayInputMapContext,0);
		}
	}
}

void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Triggered, this,&ACPlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&ACPlayerCharacter::HandleLook);
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&ACPlayerCharacter::HandleMove);

		// Codex: Lesson 35 - Bind every configured ability action with its enum
		// value so the ASC receives the same InputID used by GiveAbility.
		for (const TPair<ECAbilityInputID, UInputAction*>& InputActionPair : GameplayAbilityInputActions)
		{
			if (InputActionPair.Value)
			{
				EnhancedInputComponent->BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
			}
		}
	}
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID)
{
	if (!GetAbilitySystemComponent())
	{
		return;
	}

	const bool bPressed = InputActionValue.Get<bool>();
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed(static_cast<int32>(InputID));
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased(static_cast<int32>(InputID));
	}
}

void ACPlayerCharacter::HandleLook(const FInputActionValue& LookActionValue)
{
	FVector2D InputValue = LookActionValue.Get<FVector2D>();

	AddControllerYawInput(InputValue.X);
	AddControllerPitchInput(InputValue.Y);
}

void ACPlayerCharacter::HandleMove(const FInputActionValue& MoveActionValue)
{
	FVector2D InputValue = MoveActionValue.Get<FVector2D>();
	InputValue.Normalize();

	AddMovementInput(GetMoveForwardVector()*InputValue.Y + GetLookRightVector()* InputValue.X);
	
}

FVector ACPlayerCharacter::GetMoveForwardVector() const
{
	return FVector::CrossProduct(GetLookRightVector(),FVector::UpVector);
}

FVector ACPlayerCharacter::GetLookRightVector() const
{
	return CameraComp->GetRightVector();
}

FVector ACPlayerCharacter::GeLookForwardVector() const
{
	return CameraComp->GetForwardVector();
}
