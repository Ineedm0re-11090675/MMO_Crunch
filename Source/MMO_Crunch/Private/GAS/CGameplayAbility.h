#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "CGameplayAbility.generated.h"

UCLASS()
class UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UCGameplayAbility();	
protected:
	UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultsFromSweepLocationTargetData(FGameplayAbilityTargetDataHandle& TargetDataHandle,float SphereRadius =20.f,ETeamAttitude::Type TargetTeam =ETeamAttitude::Hostile,bool bShowDebugSphere = false,bool bIgnoreSelf = true) const;
	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebugSphere() const { return bShowDebugSphere; }
	void PushSelf(const FVector& PushForce);
	void PushTarget(AActor* Target,const FVector& PushForce);

	ACharacter* GetAvatarCharacter();
	void  ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult,TSubclassOf<UGameplayEffect> GameplayEffect,int Level = 1);
private: 
	UPROPERTY(EditDefaultsOnly,Category = "Debug")
	bool bShowDebugSphere = false;

	UPROPERTY()
	ACharacter* AvatarCharacter;
};
