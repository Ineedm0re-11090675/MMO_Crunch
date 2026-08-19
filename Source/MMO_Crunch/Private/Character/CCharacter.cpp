#include "CCharacter.h"

#include "Components/WidgetComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/OverHeadStatsGauge.h"

void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadStatsWidget(); 
}

ACCharacter::ACCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>(FName("CAttribute Set"));

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidget->SetupAttachment(GetRootComponent());
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

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
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
