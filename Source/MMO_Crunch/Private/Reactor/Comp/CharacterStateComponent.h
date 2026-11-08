#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CharacterStateComponent.generated.h"

class UCAbilitySystemComponent;
class UCAttributeSet;
class APlayerController;
class UMapleCombatManagerComponent;
struct FOnAttributeChangeData;

// Codex: Relay legacy state-tag transitions to character-specific presentation without binding the character to the old ASC.
DECLARE_MULTICAST_DELEGATE_TwoParams(FCharacterLegacyStateTagChanged, FGameplayTag, int32);

// Codex: Added Crunch-owned bridge for Health/Mana threshold tags and MapleCombat death lifecycle.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateComponent();

	// Codex: Bind after ActorInfo is initialized and before legacy startup effects are applied.
	void InitializeFromLegacyAbilitySystem(
		UCAbilitySystemComponent* InAbilitySystemComponent,
		UCAttributeSet* InAttributeSet);
	// Codex: Bind MapleCombat death callbacks so the component can cache the controller and request GameMode respawn.
	void InitializeMapleCombatLifecycle();
	// Codex: Rebind MapleCombat after its base BeginPlay lookup so mixed ASC interfaces resolve to Maple ASC.
	void BindMapleCombatAbilitySystem();
	// Codex: Let the state component resolve and clear its legacy ASC death state when a pooled character is reused.
	void ResetLegacyDeathState() const;
	// Codex: Own the combined legacy and MapleCombat death-state query for Crunch characters.
	bool IsDead() const;
	// Codex: Keep focus state beside the legacy ASC tag listener that updates it.
	bool IsInFocusMode() const { return bIsInFocusMode; }
	// Codex: Let ACCharacter preserve movement, camera, and stun responses while this component owns ASC tag subscriptions.
	FCharacterLegacyStateTagChanged OnLegacyStateTagChanged;

protected:
	//Release delegate bindings when the owning character ends play.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Codex: Keep MapleCombat delegate bindings local to this state/lifecycle component.
	TWeakObjectPtr<UMapleCombatManagerComponent> BoundMapleCombatManager;
	TWeakObjectPtr<APlayerController> CachedRespawnPlayerController;
	bool bMapleRespawnRequested = false;

	//Keep the old ASC as a temporary data source until damage effects migrate to MapleCombat.
	TWeakObjectPtr<UCAbilitySystemComponent> BoundLegacyAbilitySystemComponent;
	// Codex: Keep old resource-ratio callbacks bound to their Crunch AttributeSet during migration.
	TWeakObjectPtr<UCAttributeSet> BoundLegacyAttributeSet;
	// Codex: Store Focus as component-owned state instead of duplicating the old ASC tag in ACCharacter.
	bool bIsInFocusMode = false;

	//Preserve legacy Health/Mana tags and death-event behavior outside the ASC class.
	void HandleHealthChanged(const FOnAttributeChangeData& ChangedData);
	void HandleManaChanged(const FOnAttributeChangeData& ChangedData);
	// Codex: Rescale current resource values when the old ASC changes their maximum values.
	void HandleMaxHealthChanged(const FOnAttributeChangeData& ChangedData);
	void HandleMaxManaChanged(const FOnAttributeChangeData& ChangedData);
	// Codex: Keep the legacy MoveSpeed attribute listener with the rest of the ASC-to-character state bridge.
	void HandleMoveSpeedChanged(const FOnAttributeChangeData& ChangedData);
	// Codex: Apply the current legacy MoveSpeed value to this character's movement component.
	void ApplyMoveSpeed(float NewMoveSpeed) const;
	// Codex: Update Focus state and relay legacy combat tags to the character-facing behavior hooks.
	void HandleLegacyStateTagChanged(const FGameplayTag Tag, int32 NewCount);
	//Cache the owning player controller before MapleCombat detaches it from the dying Pawn.
	void HandleMapleCombatStartedDying(AActor* DyingActor);
	//Route completed MapleCombat deaths through ACGameMode for replacement-Pawn respawn.
	void HandleMapleCombatFinishedDying(AActor* DyingActor);
	void SetLooseTag(UCAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& Tag, bool bShouldHaveTag) const;
};
