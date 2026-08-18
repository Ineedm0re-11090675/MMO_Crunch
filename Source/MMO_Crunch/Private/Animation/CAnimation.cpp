#include "CAnimation.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UCAnimation::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter->GetCharacterMovement();
		BodyPreRot = OwnerCharacter->GetActorRotation();
	}
}
void UCAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter)
	{
		Speed = OwnerCharacter->GetVelocity().Size();

		const FRotator CurRot = OwnerCharacter->GetActorRotation();

		const FRotator DeltaRot =
			UKismetMathLibrary::NormalizedDeltaRotator(CurRot, BodyPreRot);

		if (DeltaSeconds > KINDA_SMALL_NUMBER)
		{
			YawSpeed = DeltaRot.Yaw / DeltaSeconds;
		}

		BodyPreRot = CurRot;

		SmoothedYawSpeed = FMath::FInterpTo(
			SmoothedYawSpeed,
			YawSpeed,
			DeltaSeconds,
			YawSpeedSmoothLerpSpeed
		);
		FRotator ControllerRot =OwnerCharacter->GetBaseAimRotation();
		LookRotOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControllerRot, BodyPreRot);
	}
	if (OwnerMovementComp)
	{
		bIsJumping = OwnerMovementComp->IsFalling();
	}
}

void UCAnimation::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}
