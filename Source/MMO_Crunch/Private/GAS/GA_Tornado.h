#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Tornado.generated.h"

UCLASS()
class UGA_Tornado : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	// Codex: The current Tornado blueprint has no damage GE, so preserve its no-damage default until configured.
	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta = (ClampMin = "0.0"))
	float HitDamageAmount = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float HitPushSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* TornadoMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float TornadoDuration = 4.f;

	UFUNCTION()
	void TornadoDamageEventReceived(FGameplayEventData Payload);
};
