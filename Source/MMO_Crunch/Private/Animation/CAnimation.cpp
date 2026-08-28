#include "CAnimation.h"

 
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemStatics.h"
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
	UAbilitySystemComponent* OwnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TryGetPawnOwner());
	if (OwnASC)
	{
		OwnASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimingStatsAbilityTag()).AddUObject(this,&UCAnimation::OnAimTagChange );
	}
}
void UCAnimation::OnAimTagChange(const FGameplayTag Tag, int32 NewCount)
{
	bIsAiming = NewCount != 0;
}
void UCAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter)
	{
		FVector Velocity = OwnerCharacter->GetVelocity();
		Speed = Velocity.Length();
		

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
		RightSpeed = -Velocity.Dot(ControllerRot.Vector());
		ForwardSpeed = Velocity.Dot(ControllerRot.Vector().Cross(FVector::UpVector));
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

bool UCAnimation::ShouldDoFullBody() const
{
	return (Speed<=0)  && !bIsAiming;
}
