#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "CAnimation.generated.h"

UCLASS()
class UCAnimation : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const{return Speed;}

	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE bool bIsMoving() const{return Speed != 0.f;}

	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE float GetYawSpeed() const{return SmoothedYawSpeed;}

	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE bool GetIsJumping() const{return bIsJumping;}

	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE float GetLookOffsetYaw() const{return LookRotOffset.Yaw;}
	
	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE float GetLookOffsetPitch() const{return LookRotOffset.Pitch;}
	
	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE bool GetIsAiming() const{return bIsAiming;}

	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE float GetRightSpeed() const{return RightSpeed ;}

	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	FORCEINLINE float GetForwardSpeed() const{return ForwardSpeed  ;}
	UFUNCTION(BlueprintCallable,meta = (BlueprintThreadSafe))
	bool ShouldDoFullBody() const;
private:
	UPROPERTY()
	ACharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* OwnerMovementComp;

	float Speed;

	float RightSpeed;
	float ForwardSpeed;
	
	float YawSpeed;
	float SmoothedYawSpeed;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float YawSpeedSmoothLerpSpeed = 1.f;
	
	FRotator BodyPreRot;

	bool bIsJumping;
	bool bIsAiming;

	FRotator LookRotOffset;

	void OnAimTagChange(const FGameplayTag Tag, int32 NewCount);
};
