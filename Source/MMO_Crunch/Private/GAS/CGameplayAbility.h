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
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
protected:
	AActor* GetAimTarget(float AimDistance,ETeamAttitude::Type TeamAttitude) const;
	UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultsFromSweepLocationTargetData(FGameplayAbilityTargetDataHandle& TargetDataHandle,float SphereRadius =20.f,ETeamAttitude::Type TargetTeam =ETeamAttitude::Hostile,bool bShowDebugSphere = false,bool bIgnoreSelf = true) const;
	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebugSphere() const { return bShowDebugSphere; }
	void PushSelf(const FVector& PushForce);
	void PushTarget(AActor* Target,const FVector& PushForce);
	void PushTargets(const TArray<AActor*>Actors,const FVector& PushForce);
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle,const FVector& PushForce);
	void PushTargetsFromLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle,const FVector& FromLocation , float PushSpeed);
	void PushTargetsFromLocation(const TArray<AActor*>& Targets,const FVector& FromLocation , float PushSpeed);
	void PushTargetsFromOwnerLocation(const TArray<AActor*>& Targets, float PushSpeed);
	void PushTargetsFromOwnerLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float PushSpeed);
	
	void PlayMontageLocally(UAnimMontage* MontageToPlay);
	void StopMontageAfterCurrentSection(UAnimMontage* MontageToStop);
	ACharacter* GetAvatarCharacter();
	void  ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult,TSubclassOf<UGameplayEffect> GameplayEffect,int Level = 1);
	// Codex: Resolve migrated damage sources to the Maple ASC even when the ability spec still lives on the legacy ASC.
	UAbilitySystemComponent* GetMapleCombatSourceASC() const;
	// Codex: Route migrated Crunch damage through MapleCombat SetByCaller instead of legacy Health modifiers.
	void ApplyMapleCombatDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		TSubclassOf<UGameplayEffect> DamageEffect, float DamageAmount, int32 Level = 1);
	// Codex: Preserve hit context while applying a MapleCombat damage effect to a single hit actor.
	void ApplyMapleCombatDamageToHitResultActor(const FHitResult& HitResult,
		TSubclassOf<UGameplayEffect> DamageEffect, float DamageAmount, int32 Level = 1);

	FGenericTeamId GetOwnerTeamId() const;
	bool IsActorTeamAttitudeIs(const AActor* OtherActor,ETeamAttitude::Type TargetTeam) const;
	void SendLocalGameplayEvent(const FGameplayTag& Tag,const FGameplayEventData& Data);
private: 
	// Codex: Share ASC resolution and spec application between migrated target-data and hit-result paths.
	void ApplyMapleCombatDamageToActor(AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffect,
		float DamageAmount, int32 Level, const FHitResult* ContextHitResult);

	UPROPERTY(EditDefaultsOnly,Category = "Debug")
	bool bShowDebugSphere = false;

	UPROPERTY()
	ACharacter* AvatarCharacter;
};

inline FGenericTeamId UCGameplayAbility::GetOwnerTeamId() const
{
	IGenericTeamAgentInterface* GenericTeamAgentInterface  = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	if (GenericTeamAgentInterface)
	{
		return GenericTeamAgentInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}
