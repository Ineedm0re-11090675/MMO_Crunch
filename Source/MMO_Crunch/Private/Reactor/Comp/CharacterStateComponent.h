#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CharacterStateComponent.generated.h"

class UCAbilitySystemComponent;
class APlayerController;
class UMapleCombatManagerComponent;
struct FOnAttributeChangeData;

// Codex: Added Crunch-owned bridge for Health/Mana threshold tags and MapleCombat death lifecycle.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateComponent();

	// Codex: Bind after ActorInfo is initialized and before legacy startup effects are applied.
	void InitializeFromLegacyAbilitySystem(UCAbilitySystemComponent* InAbilitySystemComponent);
	// Codex: Bind MapleCombat death callbacks so the component can cache the controller and request GameMode respawn.
	void InitializeMapleCombatLifecycle();

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

	//Preserve legacy Health/Mana tags and death-event behavior outside the ASC class.
	void HandleHealthChanged(const FOnAttributeChangeData& ChangedData);
	void HandleManaChanged(const FOnAttributeChangeData& ChangedData);
	//Cache the owning player controller before MapleCombat detaches it from the dying Pawn.
	void HandleMapleCombatStartedDying(AActor* DyingActor);
	//Route completed MapleCombat deaths through ACGameMode for replacement-Pawn respawn.
	void HandleMapleCombatFinishedDying(AActor* DyingActor);
	void SetLooseTag(UCAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& Tag, bool bShouldHaveTag) const;
};
