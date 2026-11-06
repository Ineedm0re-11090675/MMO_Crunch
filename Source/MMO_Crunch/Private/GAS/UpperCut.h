#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "UpperCut.generated.h"

// Codex: Scope combo hit data to UpperCut now that the shared legacy GE definition is unused elsewhere.
USTRUCT(BlueprintType)
struct FUpperCutComboDamageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector PushVelocity = FVector::ZeroVector;

	// Codex: Store positive MapleCombat damage independently from the removed legacy Health GE.
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float DamageAmount = 1.0f;
};

UCLASS()
class UUpperCut : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UUpperCut();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UPROPERTY(EditDefaultsOnly,Category = "Combo")
	TMap<FName,FUpperCutComboDamageData> DamageMap;

	
	UPROPERTY(EditDefaultsOnly,Category = "Animation")
	UAnimMontage* UpperCutMontage;

	// Codex: Keep UpperCut's launch amount independent from the removed legacy GE; MapleCombat owns damage execution.
	UPROPERTY(EditDefaultsOnly, Category = "Launch", meta = (ClampMin = "0.0"))
	float LaunchDamageAmount = 1.0f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Launch")
	float UppercutLaunchSpeed = 1000.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Launch")
	float UppercutComboHoldSpeed = 100.f;
	
	static FGameplayTag GetUpperCutTag();

	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);

	
	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData Data);

	UFUNCTION()
	void HandleComboCommitEvent(FGameplayEventData Data);

	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData Data);
	
	FName NextComboName;

	const FUpperCutComboDamageData* GetDamageDefFromCurrentCombo() const;
};
