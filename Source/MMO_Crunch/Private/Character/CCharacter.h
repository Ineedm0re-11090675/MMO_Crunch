#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GAS/CGameplayAbility.h"
#include "GAS/CGameplayAbilityTypes.h"
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

	const TMap<ECAbilityInputId,TSubclassOf<class UGameplayAbility>>& GetAbility() const;
protected:
	UPROPERTY(VisibleDefaultsOnly,Category = "Gameplay Effects")
	class UCAbilitySystemComponent* CAbilitySystemComponent;
	UPROPERTY()
	class UCAttributeSet* CAttributeSet;
	/*
	 *UI
	*/
	void UpgradeAbilityWithInputID(ECAbilityInputId InputID);
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

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& GameplayTag,const FGameplayEventData& GameplayEventData);
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
	void HandleStunTagChanged(const FGameplayTag Tag,int32 NewCount);
	void HandleAimingTagChanged(const FGameplayTag Tag,int32 NewCount);
	void SetIsAiming(bool bIsAiming);
	virtual void OnAimChange(bool bIsAiming);
	
	void StartDeathSequence();
	void Respawn();

	// for playCharacter
	virtual void OnDeath();
	virtual void OnRespawn();

	/*
	 *Stun
	 */
	UPROPERTY(EditDefaultsOnly,Category= "Stun")
	UAnimMontage* StunMontage;

	virtual void OnStun();
	virtual void OnRecoveryFromStun();
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


	/*
	*Attribute Update
	*/
	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);
	void MaxHealthUpdated(const FOnAttributeChangeData& Data);
	void MaxManaUpdated(const FOnAttributeChangeData& Data); 
};


