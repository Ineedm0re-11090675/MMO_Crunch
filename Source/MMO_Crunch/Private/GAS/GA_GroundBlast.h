#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CombatAbility_Cast.h"
#include "Interfaces/Animation/CombatAnimationMontageProviderInterface.h"
#include "GA_GroundBlast.generated.h"

class AGameplayAbilityTargetActor;
class ATargetActor_GroundPick;

// Codex: Use MapleCombat's native Cast lifecycle while retaining Crunch's existing skill data and animation provider contract.
UCLASS()
class UGA_GroundBlast : public UCombatAbility_Cast, public ICombatAnimationMontageProviderInterface
{
	GENERATED_BODY()

public:
	UGA_GroundBlast(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Codex: Reuse the existing GroundPick blueprint class and apply Crunch's radius, range, and debug settings through Maple's actor modifier callback.
	virtual void ModifyActor_Implementation(AGameplayAbilityTargetActor* TargetActor) override;
	virtual TSubclassOf<AGameplayAbilityTargetActor> GetTargetingActorClass_Implementation() const override;

	// Codex: Feed the existing Blueprint montage property into MapleCombat's animation provider.
	virtual UAnimMontage* GetAnimationMontage_Implementation(FGameplayTagContainer AbilityTags) const override;
	virtual float GetCastEffectLevel_Implementation() const override;

protected:
	// Codex: Extend Maple's target-acquired commit and effect application with GroundBlast's existing push and cue behavior.
	virtual void ApplyEffectToTargets(FGameplayAbilityTargetDataHandle TargetDataHandle,
		FGameplayEffectSpecHandle EffectSpecHandle) override;
	virtual void ApplyCastMagnitudesToSpec(const FGameplayEffectSpecHandle& EffectSpecHandle,
		int32 TargetCount) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* GroundBlastMontage;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* CastMontage;

	UPROPERTY(EditDefaultsOnly, Category="Cue")
	FGameplayTag BlastGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, Category="Target")
	TSubclassOf<ATargetActor_GroundPick> TargetActorClass;

	UPROPERTY(EditDefaultsOnly, Category="Target")
	float TargetAreaRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category="Target")
	float TargetTraceRange = 2000.f;

	// Codex: Preserve the debug setting previously inherited from UCGameplayAbility for existing Blueprint defaults.
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bShowDebugSphere = false;

	// Codex: Keep Maple damage magnitude separate from the legacy GE reference used by FGenericDamageEffectDef.
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float GroundBlastDamageAmount = 1.0f;

	// Codex: Keep the launch vector as Crunch skill data while MapleCombat owns damage execution.
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FVector GroundBlastPushVelocity = FVector(0.0f, 0.0f, 1000.0f);

	// Codex: Route the launch trigger to the Maple ASC that receives GAP_Launched from DA_CrunchGAS.
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushVelocity);
};
