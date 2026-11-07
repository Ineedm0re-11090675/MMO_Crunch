#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/MapleGASCharacter.h"
#include "Interfaces/CombatSystemInterface.h"
#include "Widget/RenderActorTargetInterface.h"
#include "CCharacter.generated.h"

/*
 **
 */


// 前置声明 Character 成长组件。
class UCharacterProgressionComponent;
// Codex: Keep Crunch-specific attribute threshold tags out of the legacy ASC.
class UCharacterStateComponent;


UCLASS()
class ACCharacter : public AMapleGASCharacter,
					public ICombatSystemInterface,
					public IGenericTeamAgentInterface,
					public IRenderActorTargetInterface
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
	// Codex: Expose the inherited Maple ASC explicitly while legacy callers still use the existing interface.
	UAbilitySystemComponent* GetMapleAbilitySystemComponent() const;

	const TMap<ECAbilityInputId,TSubclassOf<class UGameplayAbility>>& GetAbility() const;

	virtual FVector GetCaptureLocalPosition() const override;
	virtual FRotator GetCaptureLocalRotation() const override;
private:
	UPROPERTY(EditDefaultsOnly,Category= "Capture")
	FVector HeadshotLocalCapturePosition;
	UPROPERTY(EditDefaultsOnly,Category= "Capture")
	FRotator HeadshotLocalCaptureRotation;
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

	FORCEINLINE bool GetIsInFocusMode() const { return bIsInFocusMode; }
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& GameplayTag,const FGameplayEventData& GameplayEventData);
private:
	FTransform MeshRelativeTransform;
	
	void BindGASChangedDelegate();
	
	void HandleStunTagChanged(const FGameplayTag Tag,int32 NewCount);
	void HandleAimingTagChanged(const FGameplayTag Tag,int32 NewCount);
	void HandleFocusTagChanged(const FGameplayTag Tag,int32 NewCount);
	bool bIsInFocusMode = false;
	void SetIsAiming(bool bIsAiming);
	virtual void OnAimChange(bool bIsAiming);
	
	// Codex: MapleCombat and UCharacterStateComponent own death presentation and respawn lifecycle.

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

	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;


	/*
	*Attribute Update
	*/
	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);
	
private:
	/*
	 **************Reactor************
	 */
	// Character 持有项目成长规则组件。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCharacterProgressionComponent> CharacterProgressionComponent;
	// Codex: Own Health/Mana threshold state while the legacy attributes are being migrated.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCharacterStateComponent> CharacterStateComponent;
public:
	//UI/输入请求升级；具体规则交给组件，实际 spec 修改交给 ASC。
	UFUNCTION(BlueprintCallable, Category="Crunch|Progression")
	void RequestAbilityUpgrade(TSubclassOf<UGameplayAbility> AbilityClass);

	UFUNCTION(Server, Reliable)
	void Server_RequestAbilityUpgrade(TSubclassOf<UGameplayAbility> AbilityClass);
};


