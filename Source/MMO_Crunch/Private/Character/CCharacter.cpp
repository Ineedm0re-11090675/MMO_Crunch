#include "CCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widget/OverHeadStatsGauge.h"

void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadStatsWidget();
	MeshRelativeTransform =  GetMesh()->GetRelativeTransform(); 
}

ACCharacter::ACCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>(FName("CAttribute Set"));

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidget->SetupAttachment(GetRootComponent());

	BindGASChangedDelegate();
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

void ACCharacter::DeathMontageFinished()
{
	SetRagDollEnabled(true);
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

void ACCharacter::StartDeathSequence()
{
	OnDeath();
	
	PlayDeathMontage();
	SetStatsGaugeEnabled(false);

	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void ACCharacter::Respawn()
{
	OnRespawn();


	SetRagDollEnabled(false);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatsGaugeEnabled(true);

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
