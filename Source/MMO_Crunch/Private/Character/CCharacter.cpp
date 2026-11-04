#include "CCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/MapleCombatManagerComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"
#include "AbilitySystem/MapleGASAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MMO_Crunch/MMO_Crunch.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Widget/OverHeadStatsGauge.h"
#include "Reactor/Comp/CharacterProgressionComponent.h"
// Codex: Character-owned state component now handles legacy Health/Mana threshold tags.
#include "Reactor/Comp/CharacterStateComponent.h"
// Codex: Read the MapleCombat dead-state tag for legacy character queries during the ASC migration.
#include "MapleCombatTags.h"



void ACCharacter::BeginPlay()
{
	// Codex: Bind before inherited Maple GAS startup effects can trigger a death flow.
	if (CharacterStateComponent)
	{
		CharacterStateComponent->InitializeMapleCombatLifecycle();
	}
	Super::BeginPlay();
	// Codex: MapleCombat binds through IAbilitySystemInterface, which still resolves to the legacy ASC on ACCharacter.
	// Rebind the manager to the explicit Maple ASC after component BeginPlay has performed its default binding.
	if (UMapleCombatManagerComponent* CombatManager = FindComponentByClass<UMapleCombatManagerComponent>())
	{
		if (UAbilitySystemComponent* MapleASC = GetMapleAbilitySystemComponent())
		{
			CombatManager->SetAbilitySystemComponent(MapleASC);
		}
	}

	if (HasAuthority() && CharacterProgressionComponent)
	{
		CharacterProgressionComponent->InitializeFromASC(GetMapleAbilitySystemComponent());
	}
	// Codex: The previous duplicate Super::BeginPlay call was removed; continue actor setup once.
	ConfigureOverHeadStatsWidget();
	MeshRelativeTransform =  GetMesh()->GetRelativeTransform();

	
	/*AI detect 三层
	 *
	 *Character设置stimuli 可以被感知
	 *
	 *AI Perception Component 可以挂反馈，挂 Sight
	 *
	 *Sight挂参数
	 */
	
	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

ACCharacter::ACCharacter()
{
	// 在 ACCharacter 上创建 Character 成长组件。
	CharacterProgressionComponent =
		CreateDefaultSubobject<UCharacterProgressionComponent>(TEXT("Character Progression Component"));
	// Codex: Extract legacy Health/Mana state updates from UCAbilitySystemComponent.
	CharacterStateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("Character State Component"));

	

	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SpringArm,ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target,ECR_Ignore);
	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>(FName("CAttribute Set"));

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidget->SetupAttachment(GetRootComponent());

	BindGASChangedDelegate();

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception Stimuli Source Component");
}

void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
	// Codex: Bind before startup effects so the state component observes initial Health/Mana changes.
	if (CharacterStateComponent)
	{
		CharacterStateComponent->InitializeFromLegacyAbilitySystem(CAbilitySystemComponent);
	}
	CAbilitySystemComponent->ServerSideInit();
}

void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
	// Codex: Bind the client-side legacy ASC for attribute replication callbacks.
	if (CharacterStateComponent)
	{
		CharacterStateComponent->InitializeFromLegacyAbilitySystem(CAbilitySystemComponent);
	}
}

bool ACCharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalPlayerController();
}

void ACCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	/*
	 *AI
	 */
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

void ACCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCharacter, TeamId);
}

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

UAbilitySystemComponent* ACCharacter::GetMapleAbilitySystemComponent() const
{
	// Codex: The Maple base class may clear its cached pointer after the mixed legacy interface lookup; resolve the Maple component by class.
	return FindComponentByClass<UMapleGASAbilitySystemComponent>();
}

const TMap<ECAbilityInputId, TSubclassOf<UGameplayAbility>>& ACCharacter::GetAbility() const
{
	return CAbilitySystemComponent->GetAbility();
}

FVector ACCharacter::GetCaptureLocalPosition() const
{
	return HeadshotLocalCapturePosition;
}

FRotator ACCharacter::GetCaptureLocalRotation() const
{
	return HeadshotLocalCaptureRotation;
} 

void ACCharacter::HandleStunTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;

	if (NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}else
	{
		OnRecoveryFromStun();
		StopAnimMontage(StunMontage);
	}
}

void ACCharacter::HandleAimingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetIsAiming(NewCount !=0);
}

void ACCharacter::HandleFocusTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bIsInFocusMode = NewCount >0;
	
}

void ACCharacter::OnAimChange(bool bIsAiming)
{
	//Override on child;
}

void ACCharacter::SetIsAiming(bool bIsAiming)
{
	bUseControllerRotationYaw = bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	OnAimChange(bIsAiming);
}

void ACCharacter::OnStun()
{
}

void ACCharacter::OnRecoveryFromStun()
{
}
void ACCharacter::ConfigureOverHeadStatsWidget()
{
	if (OverHeadWidget == nullptr)return;
	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidget->SetHiddenInGame(true);
		return;
	}
	UOverHeadStatsGauge* OverHeadStatsGauge  = Cast<UOverHeadStatsGauge>(OverHeadWidget->GetUserWidgetObject());
	if (OverHeadStatsGauge)
	{
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidget->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(OverHeadStatsGaugeUpdateTimer);
		GetWorldTimerManager().SetTimer(OverHeadStatsGaugeUpdateTimer,this,&ACCharacter::UpdateOverHeadStatsGauge,HSGVisibleCheckGap,true);
	}
}

void ACCharacter::UpdateOverHeadStatsGauge()
{
	APawn* LocalCharacter = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalCharacter)
	{
		const float DisSquared = FVector::DistSquared(GetActorLocation(), LocalCharacter->GetActorLocation());
		OverHeadWidget->SetHiddenInGame(DisSquared > HSGVisibleRangeSquared);
	}
}



bool ACCharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag& GameplayTag,
	const FGameplayEventData& GameplayEventData)
{
	return true;
}

void ACCharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag& GameplayTag,
                                                                const FGameplayEventData& GameplayEventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,GameplayTag,GameplayEventData);
}

bool ACCharacter::IsDead() const
{
	// Codex: Keep legacy consumers aware of MapleCombat deaths while the old ASC still owns other state tags.
	if (IsValid(CAbilitySystemComponent) &&
		CAbilitySystemComponent->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeathStatsAbilityTag()))
	{
		return true;
	}

	const UAbilitySystemComponent* MapleAbilitySystemComponent = GetMapleAbilitySystemComponent();
	return IsValid(MapleAbilitySystemComponent) && MapleAbilitySystemComponent->HasMatchingGameplayTag(Tag_Combat_State_Dead);
}

void ACCharacter::SpawnImmediately()
{
	if (HasAuthority())
	{
		// Codex: Clear the legacy death tag mirrored by CharacterStateComponent when pooled Minions are reused.
		CAbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UCAbilitySystemStatics::GetDeathStatsAbilityTag());
		CAbilitySystemComponent->RemoveLooseGameplayTag(UCAbilitySystemStatics::GetDeathStatsAbilityTag());
		CAbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeathStatsAbilityTag()));
	}
}

void ACCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

FGenericTeamId ACCharacter::GetGenericTeamId() const
{
	return TeamId;
}

void ACCharacter::OnRep_TeamID()
{
	//Override On Child
}

void ACCharacter::BindGASChangedDelegate()
{
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatsAbilityTag()).AddUObject(this,&ACCharacter::HandleStunTagChanged);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimingStatsAbilityTag()).AddUObject(this,&ACCharacter::HandleAimingTagChanged);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetFocusTag()).AddUObject(this,&ACCharacter::HandleFocusTagChanged);
		
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMoveSpeedAttribute()).AddUObject(this,&ACCharacter::MoveSpeedUpdated);
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxHealthAttribute()).AddUObject(this,&ACCharacter::MaxHealthUpdated);
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxManaAttribute()).AddUObject(this,&ACCharacter::MaxManaUpdated);
	}
}

void ACCharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void ACCharacter::MaxHealthUpdated(const FOnAttributeChangeData& Data)
{
	if (IsValid(CAttributeSet))
	{
		CAttributeSet->RescaleHealth();
	}
}

void ACCharacter::MaxManaUpdated(const FOnAttributeChangeData& Data)
{
	if (IsValid(CAttributeSet))
	{
		CAttributeSet->RescaleMana();
	}
}
// 客户端请求经 Character RPC 到服务器，服务器再委托给成长组件。
void ACCharacter::RequestAbilityUpgrade(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass || !CharacterProgressionComponent)
	{
		return;
	}

	if (HasAuthority())
	{
		CharacterProgressionComponent->TryUpgradeAbility(AbilityClass);
	}
	else
	{
		Server_RequestAbilityUpgrade(AbilityClass);
	}
}

// 服务器执行升级；组件会校验技能白名单和升级点。
void ACCharacter::Server_RequestAbilityUpgrade_Implementation(
	TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (CharacterProgressionComponent)
	{
		CharacterProgressionComponent->TryUpgradeAbility(AbilityClass);
	}
}
