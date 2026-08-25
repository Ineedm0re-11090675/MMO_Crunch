#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "CCharacter.generated.h"
/*
 **
 */
UCLASS()
class ACCharacter : public ACharacter,public IAbilitySystemInterface ,public IGenericTeamAgentInterface
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;	
public:
	ACCharacter();

	void ServerSideInit();
	void ClientSideInit();

	bool IsLocallyControlledByPlayer() const;

	virtual void PossessedBy(AController* NewController) override;
	/*
	 *Gameplay Ability
	*/
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	UPROPERTY(VisibleDefaultsOnly,Category = "Gameplay Effects")
	class UCAbilitySystemComponent* CAbilitySystemComponent;
	UPROPERTY()
	class UCAttributeSet* CAttributeSet;
	/*
	 *UI
	*/
private:
	UPROPERTY(VisibleDefaultsOnly,Category = "UI")
	class UWidgetComponent* OverHeadWidget;

	void ConfigureOverHeadStatsWidget();

	UPROPERTY(EditDefaultsOnly,Category= "UI")
	float  HSGVisibleRangeSquared =10000000.f;
	
	UPROPERTY(EditDefaultsOnly,Category= "UI")
	float  HSGVisibleCheckGap = 1.f;
	
	FTimerHandle OverHeadStatsGaugeUpdateTimer;

	void UpdateOverHeadStatsGauge();
	/*
	 *Death && Respawn
	 */
public:
	bool IsDead() const;

	void SpawnImmediately();
	
private:
	FTransform MeshRelativeTransform;
	
	UPROPERTY(EditDefaultsOnly,Category= "Death")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly,Category= "Death")
	float DeathMontageFinishTimeShift = -0.8f;
	FTimerHandle DeathMontageTimer;

	void DeathMontageFinished();
	void PlayDeathMontage();
	void SetRagDollEnabled(bool bEnable);

	void SetStatsGaugeEnabled(bool bEnabled);
	
	void BindGASChangedDelegate();
	void HandleDeathTagChanged(const FGameplayTag Tag,int32 NewCount);
	void StartDeathSequence();
	void Respawn();

	// for playCharacter
	virtual void OnDeath();
	virtual void OnRespawn();

	/*
	 *Team
	 */
	
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	virtual FGenericTeamId GetGenericTeamId() const override;
private:
	UPROPERTY(ReplicatedUsing=OnRep_TeamID)
	FGenericTeamId TeamId;

	UFUNCTION()
	virtual void OnRep_TeamID();
	/*
	 *AI
	 */
private:

	void SetAIPerceptionStimulusSourceEnable(bool bEnable);
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;	
};
