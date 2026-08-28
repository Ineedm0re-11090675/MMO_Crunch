#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GroundPick.generated.h"


UCLASS()
class ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:
	void SetTargetAreaRadius(float NewTargetAreaRadius);
	ATargetActor_GroundPick();
	virtual void ConfirmTargetingAndContinue() override;

	void SetTargetOption(bool bTargetFriendly,bool bTargetEnemy = true );
	FORCEINLINE void SetShouldDrawDebug(bool bDrawDebug){ bShouldDrawDebug = bDrawDebug; }
	FORCEINLINE void SetTargetTraceRange(float NewTargetTraceRange){ TargetTraceRange = NewTargetTraceRange; } 
private:
	UPROPERTY(VisibleDefaultsOnly, Category="Visual")
	UDecalComponent* DecalComponent;
	bool bShouldTargetEnemy =true;
	bool bShouldTargetFriendly = false;
	
	virtual void Tick(float DeltaTime) override;

	FVector GetTargetPoint() const;

	UPROPERTY(EditDefaultsOnly,Category = "Targetting")
	float TargetTraceRange = 2000.f;

	UPROPERTY(EditDefaultsOnly,Category = "Targetting")
	float TargetAreaRadius = 300.f;

	bool bShouldDrawDebug = false;
};
