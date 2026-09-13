#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TA_Blackhole.generated.h"

UCLASS()
class ATA_Blackhole : public AGameplayAbilityTargetActor,public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	ATA_Blackhole();
	void ConfigureBlackhole(
		float InBlackholeRange,
		float InPullSpeed,
		float InBlackholeDuration,
		const FGenericTeamId& InTeamId
		);
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const  {return TeamID;}
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPROPERTY(ReplicatedUsing= OnRep_BlackholeRangeUpdated)
	float BlackholeRange;
	UFUNCTION()
	void OnRep_BlackholeRangeUpdated();
	float PullSpeed;
	float BlackholeDuration;
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	class USphereComponent* DetectionSphereComp;

	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	UParticleSystemComponent* VFXComp;

	UFUNCTION()
	void ActorInBlackhole(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void ActorLeftBlackhole(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
