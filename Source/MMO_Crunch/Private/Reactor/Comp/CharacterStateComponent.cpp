#include "Reactor/Comp/CharacterStateComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"
#include "AbilitySystem/MapleGASAbilitySystemComponent.h"
// Codex: Access ACGameMode and the cached controller when routing completed MapleCombat deaths.
#include "FrameWork/CGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
// Codex: Resolve the manager that owns MapleCombat's death delegates, not its Damage Manager subcomponent.
#include "MapleCombatFunctionLibrary.h"
#include "MapleCombatTags.h"
#include "Components/MapleCombatManagerComponent.h"
// Codex: Keep this component passive; ACCharacter initializes it at the correct ASC lifecycle point.
UCharacterStateComponent::UCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Codex: Attach before ServerSideInit applies effects so no Health/Mana updates are missed.
void UCharacterStateComponent::InitializeFromLegacyAbilitySystem(
	UCAbilitySystemComponent* InAbilitySystemComponent,
	UCAttributeSet* InAttributeSet)
{
	if (BoundLegacyAbilitySystemComponent.Get() == InAbilitySystemComponent &&
		BoundLegacyAttributeSet.Get() == InAttributeSet)
	{
		return;
	}

	if (UCAbilitySystemComponent* PreviousAbilitySystemComponent = BoundLegacyAbilitySystemComponent.Get())
	{
		PreviousAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).RemoveAll(this);
		PreviousAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).RemoveAll(this);
		PreviousAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		PreviousAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxManaAttribute()).RemoveAll(this);
	}

	BoundLegacyAbilitySystemComponent = InAbilitySystemComponent;
	BoundLegacyAttributeSet = InAttributeSet;
	if (!IsValid(InAbilitySystemComponent))
	{
		return;
	}

	InAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(
		this, &UCharacterStateComponent::HandleHealthChanged);
	InAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(
		this, &UCharacterStateComponent::HandleManaChanged);
	// Codex: Keep max-resource listeners beside the Health/Mana state listeners that use the same legacy ASC.
	InAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxHealthAttribute()).AddUObject(
		this, &UCharacterStateComponent::HandleMaxHealthChanged);
	InAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxManaAttribute()).AddUObject(
		this, &UCharacterStateComponent::HandleMaxManaChanged);
}

// Codex: Bind once actor components exist so a fatal startup effect cannot race the death request.
void UCharacterStateComponent::InitializeMapleCombatLifecycle()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	UMapleCombatManagerComponent* CombatManager = UMapleCombatFunctionLibrary::GetCombatManagerComponent(Owner);
	if (!IsValid(CombatManager) || BoundMapleCombatManager.Get() == CombatManager)
	{
		return;
	}

	if (UMapleCombatManagerComponent* PreviousManager = BoundMapleCombatManager.Get())
	{
		PreviousManager->OnOwnerStartedDying.RemoveAll(this);
		PreviousManager->OnOwnerFinishedDying.RemoveAll(this);
	}

	BoundMapleCombatManager = CombatManager;
	CombatManager->OnOwnerStartedDying.AddUObject(this, &ThisClass::HandleMapleCombatStartedDying);
	CombatManager->OnOwnerFinishedDying.AddUObject(this, &ThisClass::HandleMapleCombatFinishedDying);
}

// Codex: Keep pooled-character legacy death-tag cleanup with the other Crunch state lifecycle rules.
void UCharacterStateComponent::ResetLegacyDeathState(UCAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!IsValid(AbilitySystemComponent) || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const FGameplayTag LegacyDeathTag = UCAbilitySystemStatics::GetDeathStatsAbilityTag();
	// Codex: Set counts to zero instead of removing possibly absent tags from the minimal replication map.
	AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(LegacyDeathTag, 0);
	AbilitySystemComponent->SetLooseGameplayTagCount(LegacyDeathTag, 0);
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(LegacyDeathTag));
}

// Codex: Centralize the migration bridge that checks death state on both ASC implementations.
bool UCharacterStateComponent::IsDead() const
{
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return false;
	}

	const UCAbilitySystemComponent* LegacyAbilitySystemComponent =
		Owner->FindComponentByClass<UCAbilitySystemComponent>();
	if (IsValid(LegacyAbilitySystemComponent) &&
		LegacyAbilitySystemComponent->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeathStatsAbilityTag()))
	{
		return true;
	}

	const UMapleGASAbilitySystemComponent* MapleAbilitySystemComponent =
		Owner->FindComponentByClass<UMapleGASAbilitySystemComponent>();
	return IsValid(MapleAbilitySystemComponent) &&
		MapleAbilitySystemComponent->HasMatchingGameplayTag(Tag_Combat_State_Dead);
}

// Codex: Remove delegate bindings to avoid callbacks into a component that is shutting down.
void UCharacterStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Codex: Unbind the MapleCombat lifecycle delegates alongside the temporary legacy attribute delegates.
	if (UMapleCombatManagerComponent* CombatManager = BoundMapleCombatManager.Get())
	{
		CombatManager->OnOwnerStartedDying.RemoveAll(this);
		CombatManager->OnOwnerFinishedDying.RemoveAll(this);
	}
	BoundMapleCombatManager.Reset();
	CachedRespawnPlayerController.Reset();

	if (UCAbilitySystemComponent* AbilitySystemComponent = BoundLegacyAbilitySystemComponent.Get())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxManaAttribute()).RemoveAll(this);
	}

	BoundLegacyAbilitySystemComponent.Reset();
	BoundLegacyAttributeSet.Reset();
	Super::EndPlay(EndPlayReason);
}

// Codex: Disable local player input and cache the controller before MapleCombat detaches the dying Pawn.
void UCharacterStateComponent::HandleMapleCombatStartedDying(AActor* DyingActor)
{
	if (DyingActor != GetOwner())
	{
		return;
	}

	// Codex: Keep Crunch-specific death cleanup here while MapleCombat owns death ability and montage playback.
	if (APawn* DyingPawn = Cast<APawn>(DyingActor))
	{
		if (UCapsuleComponent* Capsule = DyingPawn->FindComponentByClass<UCapsuleComponent>())
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	if (UWidgetComponent* OverheadWidget = DyingActor->FindComponentByClass<UWidgetComponent>())
	{
		OverheadWidget->SetHiddenInGame(true);
	}
	if (UAIPerceptionStimuliSourceComponent* StimuliSource = DyingActor->FindComponentByClass<UAIPerceptionStimuliSourceComponent>())
	{
		StimuliSource->UnregisterFromPerceptionSystem();
	}
	if (UCAbilitySystemComponent* LegacyAbilitySystemComponent = BoundLegacyAbilitySystemComponent.Get();
		IsValid(LegacyAbilitySystemComponent) && DyingActor->HasAuthority())
	{
		LegacyAbilitySystemComponent->CancelAllAbilities();

		// Codex: Mirror MapleCombat death once for remaining Crunch AI, target-selection, and reward listeners.
		const FGameplayTag LegacyDeathTag = UCAbilitySystemStatics::GetDeathStatsAbilityTag();
		if (!LegacyAbilitySystemComponent->HasMatchingGameplayTag(LegacyDeathTag))
		{
			LegacyAbilitySystemComponent->AddReplicatedLooseGameplayTag(LegacyDeathTag);
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(DyingActor, LegacyDeathTag, FGameplayEventData());
		}
	}

	APawn* DyingPawn = Cast<APawn>(DyingActor);
	APlayerController* PlayerController = DyingPawn ? Cast<APlayerController>(DyingPawn->GetController()) : nullptr;
	if (!IsValid(PlayerController))
	{
		return;
	}

	CachedRespawnPlayerController = PlayerController;
	if (PlayerController->IsLocalController())
	{
		DyingPawn->DisableInput(PlayerController);
	}
}

// Codex: Ask the authoritative GameMode to respawn the cached player after MapleCombat completes its death flow.
void UCharacterStateComponent::HandleMapleCombatFinishedDying(AActor* DyingActor)
{
	if (DyingActor != GetOwner() || !GetOwner() || !GetOwner()->HasAuthority() || bMapleRespawnRequested)
	{
		return;
	}

	APlayerController* PlayerController = CachedRespawnPlayerController.Get();
	UWorld* World = GetWorld();
	ACGameMode* GameMode = World ? World->GetAuthGameMode<ACGameMode>() : nullptr;
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Codex] MapleCombat finished dying for %s without a cached player controller."), *GetNameSafe(GetOwner()));
		return;
	}
	if (!IsValid(GameMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Codex] MapleCombat finished dying for %s without an authoritative ACGameMode."), *GetNameSafe(GetOwner()));
		return;
	}

	if (GameMode->RequestPlayerRespawn(PlayerController))
	{
		bMapleRespawnRequested = true;
	}
}

// Codex: Keep legacy Health Full/Empty tags here; MapleCombat owns fatal damage and the death lifecycle.
void UCharacterStateComponent::HandleHealthChanged(const FOnAttributeChangeData& ChangedData)
{
	UCAbilitySystemComponent* AbilitySystemComponent = BoundLegacyAbilitySystemComponent.Get();
	if (!IsValid(AbilitySystemComponent) || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	bool bFoundMaxHealth = false;
	const float MaxHealth = AbilitySystemComponent->GetGameplayAttributeValue(UCAttributeSet::GetMaxHealthAttribute(), bFoundMaxHealth);
	SetLooseTag(AbilitySystemComponent, UCAbilitySystemStatics::GetFullHealthStatsAbilityTag(),
		bFoundMaxHealth && ChangedData.NewValue >= MaxHealth);

	SetLooseTag(AbilitySystemComponent, UCAbilitySystemStatics::GetEmptyHealthStatsAbilityTag(),
		ChangedData.NewValue <= 0.0f);
}

// Codex: Move legacy Mana Full/Empty tag bookkeeping out of UCAbilitySystemComponent.
void UCharacterStateComponent::HandleManaChanged(const FOnAttributeChangeData& ChangedData)
{
	UCAbilitySystemComponent* AbilitySystemComponent = BoundLegacyAbilitySystemComponent.Get();
	if (!IsValid(AbilitySystemComponent) || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	bool bFoundMaxMana = false;
	const float MaxMana = AbilitySystemComponent->GetGameplayAttributeValue(UCAttributeSet::GetMaxManaAttribute(), bFoundMaxMana);
	SetLooseTag(AbilitySystemComponent, UCAbilitySystemStatics::GetFullManaStatsAbilityTag(),
		bFoundMaxMana && ChangedData.NewValue >= MaxMana);

	SetLooseTag(AbilitySystemComponent, UCAbilitySystemStatics::GetEmptyManaStatsAbilityTag(), ChangedData.NewValue <= 0.0f);
}

// Codex: Move legacy MaxHealth rescaling out of ACCharacter and keep it with resource-state callbacks.
void UCharacterStateComponent::HandleMaxHealthChanged(const FOnAttributeChangeData&)
{
	UCAttributeSet* AttributeSet = BoundLegacyAttributeSet.Get();
	if (IsValid(AttributeSet) && GetOwner() && GetOwner()->HasAuthority())
	{
		AttributeSet->RescaleHealth();
	}
}

// Codex: Move legacy MaxMana rescaling out of ACCharacter and keep it with resource-state callbacks.
void UCharacterStateComponent::HandleMaxManaChanged(const FOnAttributeChangeData&)
{
	UCAttributeSet* AttributeSet = BoundLegacyAttributeSet.Get();
	if (IsValid(AttributeSet) && GetOwner() && GetOwner()->HasAuthority())
	{
		AttributeSet->RescaleMana();
	}
}

// Codex: Keep loose-tag updates idempotent and match the prior ASC behavior.
void UCharacterStateComponent::SetLooseTag(
	UCAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayTag& Tag,
	const bool bShouldHaveTag) const
{
	if (!IsValid(AbilitySystemComponent) || !Tag.IsValid())
	{
		return;
	}

	if (bShouldHaveTag)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(Tag))
		{
			AbilitySystemComponent->AddLooseGameplayTag(Tag);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(Tag);
	}
}
