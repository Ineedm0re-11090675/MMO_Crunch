#include "CPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GameFramework/CharacterMovementComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Camera Spring");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComp->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

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
		UEnhancedInputLocalPlayerSubsystem* InputLocalPlayerSubsystem = OwningController->GetLocalPlayer()->GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>();
		if (InputLocalPlayerSubsystem)
		{
			InputLocalPlayerSubsystem->RemoveMappingContext(GamePlayInputMapContext);
			InputLocalPlayerSubsystem->AddMappingContext(GamePlayInputMapContext, 0);
		}
	}
}

void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleLook);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleMove);
		for (const TPair<ECAbilityInputId, UInputAction*>& InputActionPair : GameplayAbilityInputAction)
		{
			EnhancedInputComponent->BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this,
			                                   &ACPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
		}
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

	AddMovementInput(GetMoveForwardVector() * InputValue.Y + GetLookRightVector() * InputValue.X);
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& AbilityInputValue,
                                           ECAbilityInputId AbilityInputId)
{
	bool bPressed = AbilityInputValue.Get<bool>();
	/*
	*Explain
	*
		IMC
		↓
		InputAction
		↓
		Character 里的输入回调 HandleAbilityInput
		↓
		把 InputID 告诉 ASC
		↓
		ASC 根据 InputID 找到对应的 FGameplayAbilitySpec
		↓
		ASC 尝试激活这个 Spec 对应的 GameplayAbility

		需要绑定：Character ，ASC
	 */
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)AbilityInputId);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)AbilityInputId);
	}
}

void ACPlayerCharacter::OnDeath()
{
	APlayerController* OwningController = GetController<APlayerController>();
	if (OwningController)
	{
		DisableInput(OwningController);
	}
}

void ACPlayerCharacter::OnRespawn()
{
	APlayerController* OwningController = GetController<APlayerController>();
	if (OwningController)
	{
		EnableInput(OwningController);
	}
}


FVector ACPlayerCharacter::GetMoveForwardVector() const
{
	return FVector::CrossProduct(GetLookRightVector(), FVector::UpVector);
}

FVector ACPlayerCharacter::GetLookRightVector() const
{
	return CameraComp->GetRightVector();
}

FVector ACPlayerCharacter::GeLookForwardVector() const
{
	return CameraComp->GetForwardVector();
}
