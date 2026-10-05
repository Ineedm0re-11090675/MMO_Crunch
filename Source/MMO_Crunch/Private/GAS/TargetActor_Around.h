#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_Around.generated.h"

UCLASS()
class ATargetActor_Around: public AGameplayAbilityTargetActor , public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	ATargetActor_Around();
	void ConfigureDetection(float DetectionRadius, const FGenericTeamId& InTeamId, const FGameplayTag& InLocalGameplayCueTag);


	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override {return TeamId;}
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:

	UFUNCTION()
	void ActorInDetectionRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnRep_TargetDetectionRadiusReplicated();
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	UPROPERTY(VisibleDefaultsOnly, Category = "Comp")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Targeting")
	class USphereComponent* DetectionSphere;

	UPROPERTY(ReplicatedUsing = OnRep_TargetDetectionRadiusReplicated)
	float TargetDetectionRadius;


	UPROPERTY(Replicated)
	FGameplayTag LocalGameplayCueTag;
};

