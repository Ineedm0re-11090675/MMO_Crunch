#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h" 
#include "CAIController.generated.h"

UCLASS()
class ACAIController : public AAIController
{
	GENERATED_BODY()
public:
	ACAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly,Category = "AI Behaviour")
	FName TargetBlackboardKeyName = "target";
	
	UPROPERTY(EditDefaultsOnly,Category = "AI Behaviour")
	class UBehaviorTree* BehaviorTree;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAIPerceptionComponent* PerceptionComp;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAISenseConfig_Sight* SightComp;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor , FAIStimulus Stimulus);

	UFUNCTION()
	void TargetPerceptionForgottenUpdate(AActor* ForgottenActor);

	const UObject* GetCurrentTargetActor() const;

	void SetCurrentTargetActor(AActor* NewTarget);

	AActor* GetNextTarget() const;

	void ForgetActorIfDead(AActor* ActorToForget);

	void ClearAndDisableAllSenses();

	void EnableAllSenses();

	void OnPawnDeathUpdated(const FGameplayTag Tag,int32 Count);
};
