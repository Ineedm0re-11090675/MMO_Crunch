#include "CPlayerController.h"
#include "CPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widget/GameplayWidget.h"
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
	if (!IsLocalPlayerController()) return;
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
