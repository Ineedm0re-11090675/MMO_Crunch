#include "CPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CHeroAttributeSet.h"
#include "Inventory/InventoryComponent.h"
#include "MMO_Crunch/MMO_Crunch.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Camera Spring");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->ProbeChannel = ECC_SpringArm;

	CameraComp = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComp->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;

	// Rotate With Controller viewDir
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

	HeroAttributeSet = CreateDefaultSubobject<UCHeroAttributeSet>(FName("Hero Attribute Set"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
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

		EnhancedInputComponent->BindAction(UpgradeAbilityAction,ETriggerEvent::Started,this,&ACPlayerCharacter::LearnAbilityLeaderDown);
		EnhancedInputComponent->BindAction(UpgradeAbilityAction,ETriggerEvent::Completed,this,&ACPlayerCharacter::LearnAbilityLeaderUp);
		for (const TPair<ECAbilityInputId, UInputAction*>& InputActionPair : GameplayAbilityInputAction)
		{
			EnhancedInputComponent->BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this,
			                                   &ACPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
		}
		EnhancedInputComponent->BindAction(UseInventoryItemAction, ETriggerEvent::Triggered, this,&ACPlayerCharacter::UseInventoryItem);
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

void ACPlayerCharacter::LearnAbilityLeaderDown(const FInputActionValue& MoveActionValue)
{
	bIsLearnAbilityLeaderDown = true;
}

void ACPlayerCharacter::LearnAbilityLeaderUp(const FInputActionValue& MoveActionValue)
{
	bIsLearnAbilityLeaderDown = false;  
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
	if (bPressed && bIsLearnAbilityLeaderDown)
	{
		UpgradeAbilityWithInputID(AbilityInputId);
		return;
	}
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)AbilityInputId);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)AbilityInputId);
	}
	//上勾拳后，会block正常的combo，让这里触发combo 的press tag，这样可以做到连招而不是basic attack
	if (AbilityInputId == ECAbilityInputId::BasicAttack)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,UCAbilitySystemStatics::GetBasicAttackPressedTag(),FGameplayEventData());
		Server_SendGameplayEventToSelf(UCAbilitySystemStatics::GetBasicAttackPressedTag(),FGameplayEventData());
		
	}
}

void ACPlayerCharacter::UseInventoryItem(const FInputActionValue& InputValue)
{
	int Value = FMath::RoundToInt(InputValue.Get<float>());
	InventoryComponent->TryActivateItemInSlot(Value-1);
}

void ACPlayerCharacter::SetInputEnableFromPlayerController(bool bEnable)
{
	APlayerController* OwningController = GetController<APlayerController>();
	if (!OwningController)
	{
		return;
	}
	if (bEnable)
	{
		EnableInput(OwningController);
	}else
	{
		DisableInput(OwningController);
	}
}

void ACPlayerCharacter::OnDeath()
{
	SetInputEnableFromPlayerController(false);
}

void ACPlayerCharacter::OnRespawn()
{
	SetInputEnableFromPlayerController(true);
}

void ACPlayerCharacter::OnStun()
{
	if (IsDead()) return;
	SetInputEnableFromPlayerController(false);
}

void ACPlayerCharacter::OnRecoveryFromStun()
{
	if (IsDead()) return;
	SetInputEnableFromPlayerController(true);
}

void ACPlayerCharacter::OnAimChange(bool bIsAiming)
{
	if (!IsLocallyControlled())return;
	LerpCameraToLocalOffsetLocation(bIsAiming?CameraAimLocalOffset:FVector{0.f});
}

void ACPlayerCharacter::LerpCameraToLocalOffsetLocation(const FVector& Goal)
{
	GetWorldTimerManager().ClearTimer(CameraLerpTimerHandle);
	CameraLerpTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&ACPlayerCharacter::TickCameraLocalOffset,Goal));
}

void ACPlayerCharacter::TickCameraLocalOffset(FVector Goal)
{
	FVector CurrentLocalOffset =CameraComp->GetRelativeLocation();
	if (FVector::Dist(Goal,CurrentLocalOffset) < 1.f)
	{
		CameraComp->SetRelativeLocation(Goal);
		return;
	}

	float LerpAlpha = FMath::Clamp(GetWorld()->GetDeltaSeconds() * CameraLerpSpeed,0.f,1.f);

	FVector NewLocalOffset =FMath::Lerp(CurrentLocalOffset,Goal,LerpAlpha);
	CameraComp->SetRelativeLocation(NewLocalOffset);
	CameraLerpTimerHandle=GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&ACPlayerCharacter::TickCameraLocalOffset,Goal));
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
