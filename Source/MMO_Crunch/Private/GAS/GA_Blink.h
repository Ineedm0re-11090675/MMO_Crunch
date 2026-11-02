#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Blink.generated.h"

UCLASS()
class UGA_Blink : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UGA_Blink();
	static FGameplayTag GetTeleportationTag();
private:
	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* TargetingMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* TeleportMontage;


	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ATargetActor_GroundPick> GroundPickTargetActorClass;

	// Codex: The current Blink blueprint has no damage GE, so preserve its no-damage default until configured.
	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta = (ClampMin = "0.0"))
	float DamageAmount = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float BlinkLandTargetPushSpeed = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius = 1000;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float BlinkCastRange = 3000.f;


	UFUNCTION()
	void GroundPickTargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void GroundPickCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void Teleport(FGameplayEventData Payload);
	
	FGameplayAbilityTargetDataHandle BlinkTargetDataHandle;
	
};
