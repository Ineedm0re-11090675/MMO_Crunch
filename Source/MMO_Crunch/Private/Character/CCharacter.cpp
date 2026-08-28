#include "CCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "MMO_Crunch/MMO_Crunch.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Widget/OverHeadStatsGauge.h"

void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
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
	CAbilitySystemComponent->ApplyInitialEffects();
	CAbilitySystemComponent->GiveInitialAbility();
}

void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
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

const TMap<ECAbilityInputId, TSubclassOf<UGameplayAbility>>& ACCharacter::GetAbility() const
{
	return CAbilitySystemComponent->GetAbility();
}

void ACCharacter::DeathMontageFinished()
{
	if (IsDead())
	{
		SetRagDollEnabled(true);
	}else
	{
		
	}
}

void ACCharacter::PlayDeathMontage()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimer,this,&ACCharacter::DeathMontageFinished,MontageDuration +DeathMontageFinishTimeShift);
	}
}

void ACCharacter::SetRagDollEnabled(bool bEnable)
{
	if (bEnable)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void ACCharacter::SetStatsGaugeEnabled(bool bEnabled)
{
	GetWorldTimerManager().ClearTimer(OverHeadStatsGaugeUpdateTimer);
	if (bEnabled)
	{
		ConfigureOverHeadStatsWidget();
	}else
	{
		OverHeadWidget->SetHiddenInGame(true);
	}
}

void ACCharacter::BindGASChangedDelegate()
{
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeathStatsAbilityTag()).AddUObject(this,&ACCharacter::HandleDeathTagChanged);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatsAbilityTag()).AddUObject(this,&ACCharacter::HandleStunTagChanged);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimingStatsAbilityTag()).AddUObject(this,&ACCharacter::HandleAimingTagChanged);
	}
}

void ACCharacter::HandleDeathTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence();
	}else
	{
		Respawn();
	}
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
void ACCharacter::StartDeathSequence()
{
	OnDeath();

	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->CancelAllAbilities( );
	}
	PlayDeathMontage();
	SetStatsGaugeEnabled(false);

	//GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAIPerceptionStimulusSourceEnable(false);
}

void ACCharacter::Respawn()
{
	OnRespawn();


	SetRagDollEnabled(false);
	//GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatsGaugeEnabled(true);
	SetAIPerceptionStimulusSourceEnable(true);

	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor> StartSpot =GetController()->StartSpot;
		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform());
		}
	}

	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatsEffect();
	}
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

bool ACCharacter::IsDead() const
{
	return CAbilitySystemComponent->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeathStatsAbilityTag());
}

void ACCharacter::SpawnImmediately()
{
	if (HasAuthority())
	{
		CAbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeathStatsAbilityTag()));
		
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

void ACCharacter::OnDeath()
{
}

void ACCharacter::OnRespawn()
{
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

void ACCharacter::SetAIPerceptionStimulusSourceEnable(bool bEnable)
{
	if (!PerceptionStimuliSourceComponent) return;

	if (bEnable)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}else
	{
		/*
		*Character 活着 
		↓ 
		StimuliSource 已注册 
		↓ 
		AI 看到了它 
		↓ 
		PerceptionComponent 内部已经保存了一份 FAIStimulus

		死亡后 Unregister的意义是 不再作为刺激源，但是之前的刺激信息还在，所以需要在AIC里把刺激信息做过期处理
		*/
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
	
}
