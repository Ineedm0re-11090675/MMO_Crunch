#include "CPlayerController.h"
#include "CPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widget/GameplayWidget.h"
#if WITH_EDITOR
// Codex: This command deliberately uses MapleCombat's own damage GE and SetByCaller contract.
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/CombatEffect_Damage.h"
#include "AbilitySystem/MapleCombatAttributeSet.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpec.h"
#include "MapleCombatFunctionLibrary.h"
#include "MapleCombatTags.h"
#endif

#if WITH_EDITOR
// Codex: Apply self-damage through UCombatEffect_Damage so PIE can verify PendingDamage, outcomes, and death state.
void ACPlayerController::DebugMapleCombatDamage(const float DamageAmount)
{
	if (!HasAuthority())
	{
		ClientMessage(TEXT("Run DebugMapleCombatDamage on the server-side PlayerController."));
		return;
	}

	APawn* TargetPawn = GetPawn();
	if (!IsValid(TargetPawn) || DamageAmount <= 0.0f)
	{
		ClientMessage(TEXT("A possessed pawn and positive damage amount are required."));
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetPawn);
	if (!IsValid(TargetASC) || !TargetASC->GetSet<UMapleCombatAttributeSet>())
	{
		ClientMessage(TEXT("The possessed pawn does not expose a MapleCombat AttributeSet through its ASC interface."));
		return;
	}

	const UActorComponent* DamageManager = UMapleCombatFunctionLibrary::GetDamageManagerComponent(TargetPawn);
	if (!UMapleCombatFunctionLibrary::IsValidDamageManagerComponent(DamageManager))
	{
		ClientMessage(TEXT("The possessed pawn has no valid MapleCombat damage manager."));
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
		UCombatEffect_Damage::StaticClass(), 1.0f, TargetASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		ClientMessage(TEXT("Could not create the MapleCombat damage effect spec."));
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(Tag_Combat_Data_Damage, DamageAmount);
	const float HealthBefore = TargetASC->GetNumericAttribute(UMapleCombatAttributeSet::GetHealthAttribute());
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	const float HealthAfter = TargetASC->GetNumericAttribute(UMapleCombatAttributeSet::GetHealthAttribute());
	const bool bIsDead = TargetASC->HasMatchingGameplayTag(Tag_Combat_State_Dead);
	// Codex: Report whether the granted specs match MapleCombat's Combat.Ability.Death query.
	FGameplayTagContainer DeathAbilityTags;
	DeathAbilityTags.AddTag(Tag_Combat_Ability_Death);
	TArray<FGameplayAbilitySpecHandle> DeathAbilityHandles;
	TargetASC->FindAllAbilitiesWithTags(DeathAbilityHandles, DeathAbilityTags);
	int32 ActiveDeathAbilityCount = 0;
	for (const FGameplayAbilitySpecHandle& AbilityHandle : DeathAbilityHandles)
	{
		const FGameplayAbilitySpec* AbilitySpec = TargetASC->FindAbilitySpecFromHandle(AbilityHandle);
		if (AbilitySpec && AbilitySpec->ActiveCount > 0)
		{
			++ActiveDeathAbilityCount;
		}
	}

	UE_LOG(LogTemp, Display,
		TEXT("[Codex] MapleCombat self-damage: %.2f; Health %.2f -> %.2f; Dead tag: %s; Death ability specs: %d, active: %d"),
		DamageAmount, HealthBefore, HealthAfter, bIsDead ? TEXT("true") : TEXT("false"),
		DeathAbilityHandles.Num(), ActiveDeathAbilityCount);
}
#endif

void ACPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	CPlayerCharacter = Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ServerSideInit();
		CPlayerCharacter->SetGenericTeamId(TeamId);
	}
}

void ACPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	CPlayerCharacter = Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ClientSideInit();
		SpawnGameplayWidget();
	}
}

void ACPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

FGenericTeamId ACPlayerController::GetGenericTeamId() const
{
	return TeamId;
}

void ACPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACPlayerController, TeamId);
}

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputLocalPlayerSubsystem* InputLocalPlayerSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputLocalPlayerSubsystem)
	{
		InputLocalPlayerSubsystem->RemoveMappingContext(UIInputMapping);
		InputLocalPlayerSubsystem->AddMappingContext(UIInputMapping, 1);
	}
	
	UEnhancedInputComponent* EnhancedInputComponent  = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(ShopToggleInputAction,ETriggerEvent::Triggered,this,&ACPlayerController::ToggleShop);
		EnhancedInputComponent->BindAction(ToggleGameplayMenuAction,ETriggerEvent::Triggered,this, &ACPlayerController::ToggleGameplayMenu);
	}
}

void ACPlayerController::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	if (!HasAuthority()) return;
	
	CPlayerCharacter->DisableInput(this);
	Client_MatchFinished(ViewTarget,WinningTeam); 
}

void ACPlayerController::Client_MatchFinished_Implementation(AActor* ViewTarget, int WinningTeam)
{
	SetViewTargetWithBlend(ViewTarget,MatchFinishViewBlendTimeDuration);
	FString ResultMsg = "You Win!";
	if (GetGenericTeamId() != WinningTeam)
	{
		ResultMsg = "You Lose";
	}
	GameplayWidget->SetGameplayMenuTitle(ResultMsg);
	FTimerHandle ShowWinLoseStateTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ShowWinLoseStateTimerHandle,this,&ACPlayerController::ShowWinLoseState,MatchFinishViewBlendTimeDuration);
}

void ACPlayerController::SpawnGameplayWidget()
{
	// Codex: AcknowledgePossession can run for replacement Pawns; configure a HUD only when the new Pawn is valid.
	if (!IsLocalPlayerController() || !IsValid(CPlayerCharacter)) return;
	// Codex: Rebind the existing HUD to the newly possessed character instead of creating another HUD on respawn.
	if (IsValid(GameplayWidget))
	{
		GameplayWidget->ConfigureAbility(CPlayerCharacter->GetAbility());
		return;
	}

	GameplayWidget = CreateWidget<UGameplayWidget>(this,GamePlayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
		GameplayWidget->ConfigureAbility(CPlayerCharacter->GetAbility());
	}
}

void ACPlayerController::ToggleShop()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleShop();
	} 
}

void ACPlayerController::ToggleGameplayMenu()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleGameplayMene();
	}
}

void ACPlayerController::ShowWinLoseState()
{
	if (GameplayWidget)
	{
		GameplayWidget->ShowGameplayMenu();
	}	
}
