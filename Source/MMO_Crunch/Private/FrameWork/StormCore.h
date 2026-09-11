#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StormCore.generated.h"


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGoalReached, AActor* /*ViewTarget*/, int /*WinningTeam*/); 
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInfluencerCountUpdated, int /* One*/, int /*Two*/);
UCLASS()
class AStormCore : public ACharacter
{
	GENERATED_BODY()
public:
	FOnGoalReached OnGoalReached;
	FOnInfluencerCountUpdated OnInfluencerCount;
	
	AStormCore();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	float GetProgress()const ;
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
private:
	UPROPERTY(EditDefaultsOnly,Category="Move")
	UAnimMontage* ExpandMontage;

	UPROPERTY(EditDefaultsOnly,Category="Move")
	UAnimMontage* CaptureMontage;
	
	UPROPERTY(EditDefaultsOnly,Category="Move")
	float InfluenceRadius = 1000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Move")
	float MaxMoveSpeed = 500.f;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Detection")
	class USphereComponent* InfluenceRange;

	UPROPERTY(VisibleDefaultsOnly,Category="Detection")
	class UCameraComponent* ViewCam;

	UPROPERTY(VisibleDefaultsOnly,Category="Detection")
	UDecalComponent* GroundDecalComponent;
	
	UFUNCTION()
	void NewInfluenceInRange(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	UFUNCTION()
	void InfluenceLeftRange(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	
	int TeamOneInfluenceCount = 0;
	int TeamTwoInfluenceCount = 0;

	void UpdateTeamWeight();
	void UpdateGoal();

	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamOneGoal;
	
	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamTwoGoal;

	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamOneCore;
	
	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamTwoCore;

	UPROPERTY(ReplicatedUsing=OnRep_CoreToCapture)
	AActor* CoreToCapture;

	float CoreCaptureSpeed = 0.f;
	float TravelLength = 0.f;
	UFUNCTION()
	void OnRep_CoreToCapture();

	void GoalReached(int WinningTeam);

	void ExpandFinished();
	
	void CaptureCore();
	float TeamWeight = 0.f;

	UPROPERTY()
	class AAIController* OwnAIC;
};
