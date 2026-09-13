#pragma once

#include "CoreMinimal.h"
#include  "Abilities/GameplayAbilityTargetActor.h"
#include "GenericTeamAgentInterface.h"
#include "TargetActor_Line.generated.h"


UCLASS()
class ATargetActor_Line : public AGameplayAbilityTargetActor , public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	ATargetActor_Line();
	void ConfigureTargetSetting(
		float NewTargetRange,
		float NewDetectionCylinderRadius,
		float NewTargetingInterval,
		FGenericTeamId NewTargetingTeamId,
		bool bShouldDrawDebug
	);
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;

	FORCEINLINE virtual  FGenericTeamId GetGenericTeamId() const override{return TargetingTeamId;};

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override; 
private:
	UPROPERTY(Replicated)
	float TargetRange;
	
	UPROPERTY(Replicated)
	float DetectionCylinderRadius;
	
	UPROPERTY()
	float TargetingInterval;

	UPROPERTY(Replicated)
	FGenericTeamId TargetingTeamId;

	UPROPERTY()
	bool bDrawDebug;

	UPROPERTY(Replicated)
	const AActor* AvatarActor;
	
	UPROPERTY(VisibleDefaultsOnly,Category = "Component")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly,Category = "Component")
	class UNiagaraComponent* LazerVFX;

	UPROPERTY(VisibleDefaultsOnly,Category = "Component")
	class USphereComponent* TargetEndDetectionSphere;

	FTimerHandle PeriodicalTargetingTimerHandle;

	UPROPERTY(VisibleDefaultsOnly,Category = "VFX")
	FName LazerFXLengthParaName = "Length";
	
	void DoTargetCheckAndReport();
	void UpdateTargetTrack();

	bool ShouldReportActorAsTarget(const AActor* ActorToCheck) const;
};
