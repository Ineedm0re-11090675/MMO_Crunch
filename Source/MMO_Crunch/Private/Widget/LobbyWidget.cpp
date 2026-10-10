#include "LobbyWidget.h"

#include "AbilityListView.h"
#include "CharacterEntryWidget.h"
#include "Components/UniformGridPanel.h"
#include "TeamSelectionWidget.h"
#include "Character/PA_CharacterDefinition.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "Components/UniformGridSlot.h"
#include "Components/WidgetSwitcher.h"
#include "FrameWork/CAssetManager.h"
#include "FrameWork/CGameState.h"
#include "Network/CNetStatic.h"
#include "Player/CPlayerState.h"
#include "Player/LobbyPlayerController.h"
#include "CharacterDisplay.h"
#include "PlayerTeamLayoutWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ClearAndPopulateTeamSelectionSlots();
	ConfigureGameState();
	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();
	if (LobbyPlayerController)
	{
		LobbyPlayerController->OnSwitchToHeroSelection.BindUObject(this,&ULobbyWidget::SwitchToHeroSelection); 
	}
	StartHeroSelectionButton->SetIsEnabled(false);
	StartMatchButton->SetIsEnabled(false);
	StartMatchButton->OnClicked.AddDynamic(this,&ULobbyWidget::StartMatchButtonClicked);
	StartHeroSelectionButton->OnClicked.AddDynamic(this,&ULobbyWidget::StartHeroSelectionButtonClicked);

	UCAssetManager::Get().LoadCharacterDefinition(FStreamableDelegate::CreateUObject(this,&ULobbyWidget::CharacterDefinitionLoaded));
	if (CharacterSelectionTileView)
	{
		CharacterSelectionTileView->OnItemSelectionChanged().AddUObject(this,&ULobbyWidget::CharacterSelected);
	}
	SpawnCharacterDisplay();
}

void ULobbyWidget::ClearAndPopulateTeamSelectionSlots()
{
	TeamSelectionSlotGridPanel->ClearChildren();

	for (int i=0 ; i< UCNetStatic::GetPlayerCountPerTeam() * 2 ; i++)
	{
		UTeamSelectionWidget* NewSelectionSlot = CreateWidget<UTeamSelectionWidget>(this,TeamSelectionWidgetClass);
		if (NewSelectionSlot)
		{
			NewSelectionSlot->SetSlotId(i);
			UUniformGridSlot* NewGridSlot = TeamSelectionSlotGridPanel->AddChildToUniformGrid(NewSelectionSlot);
			if (NewGridSlot)
			{
				int Row = i % UCNetStatic::GetPlayerCountPerTeam();
				int Column = i < UCNetStatic::GetPlayerCountPerTeam() ? 0 : 1;

				NewGridSlot->SetRow(Row);
				NewGridSlot->SetColumn(Column);
			}
			NewSelectionSlot->OnSlotlicked.AddUObject(this, &ULobbyWidget::SelectSlot);
			TeamSelectionSlots.Add(NewSelectionSlot);
		}
	}
}

void ULobbyWidget::SelectSlot(uint8 NewSlotId)
{
	if (LobbyPlayerController)
	{
		 LobbyPlayerController->Server_RequestSlotSelectionChange(NewSlotId);
	}

	// 1（法师，8）
	// 2（战士，11）
	
}

void ULobbyWidget::ConfigureGameState()
{
	UWorld* World = GetWorld();
	if (!World) return;

	CGameState = GetWorld()->GetGameState<ACGameState>();
	if (!CGameState)
	{
		World->GetTimerManager().SetTimer(ConfigureGameStateTimerHandle,this,&ULobbyWidget::ConfigureGameState,1.f);
	}
	else
	{
		CGameState->OnPlayerSelectionUpdated.AddUObject(this,&ULobbyWidget::UpdatePlayerSelectionDisplay);
		UpdatePlayerSelectionDisplay(CGameState->GetPlayerSelectionArray());
	}
}

void ULobbyWidget::UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UTeamSelectionWidget* NewSelectionSlot : TeamSelectionSlots)
	{
		NewSelectionSlot->UpdateSlotInfo("Empty");
	}
	for (UUserWidget* CharacterEntryAsWidget : CharacterSelectionTileView->GetDisplayedEntryWidgets())
	{
		if (UCharacterEntryWidget* CharacterEntryWidget = Cast<UCharacterEntryWidget>(CharacterEntryAsWidget))
		{
			//初始化：无选择
			CharacterEntryWidget->SetSelected(false);
		}
	}

	for (const FPlayerSelection& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid()) continue;

		TeamSelectionSlots[PlayerSelection.GetSlot()]->UpdateSlotInfo(PlayerSelection.GetPlayerNickName());

		UCharacterEntryWidget* CharacterEntryWidget = CharacterSelectionTileView->GetEntryWidgetFromItem<UCharacterEntryWidget>(PlayerSelection.GetCharacterDefinition());
		if (CharacterEntryWidget)
		{
			CharacterEntryWidget->SetSelected(true);
		}

		if (PlayerSelection.IsForPlayer(GetOwningPlayerState()))
		{
			UpdateCharacterDisplay(PlayerSelection);
		}
		
	}
	if (CGameState)
	{
		StartHeroSelectionButton->SetIsEnabled(CGameState->CanStartHeroSelection());
		StartMatchButton->SetIsEnabled(CGameState->CanStartMatch());
	}
	if (PlayerTeamLayoutWidget)
	{
		PlayerTeamLayoutWidget->UpdatePlayerSelection(PlayerSelections);
	}
}

void ULobbyWidget::StartHeroSelectionButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_StartHeroSelection();
	}
}

void ULobbyWidget::StartMatchButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_RequestStartMatch();
	}
}

void ULobbyWidget::SwitchToHeroSelection()
{
	MainSwitcher->SetActiveWidget(HeroSelectionRoot);
}

void ULobbyWidget::CharacterDefinitionLoaded()
{
	TArray<UPA_CharacterDefinition*> LoadedCharacterDefinitions;
	if (UCAssetManager::Get().GetLoadedCharacterDefinition(LoadedCharacterDefinitions))
	{
		CharacterSelectionTileView->SetListItems(LoadedCharacterDefinitions);
	}
}

void ULobbyWidget::CharacterSelected(UObject* SelectedUObject)
{
	if (!CPlayerState)
	{
		CPlayerState = GetOwningPlayerState<ACPlayerState>();
	}
	
	if (!CPlayerState) return;
	
	
	if (const UPA_CharacterDefinition* CharacterDefinition = Cast<UPA_CharacterDefinition>(SelectedUObject))
	{
		CPlayerState->Server_SetSelectedCharacterDefinition(CharacterDefinition);
	}
}

void ULobbyWidget::SpawnCharacterDisplay()
{
	if (CharacterDisplay) return;
	if (!CharacterDisplayClass) return;

	FTransform CharacterDisplayTransform = FTransform::Identity;
	AActor* ActorStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
	//寻找第一个PlayerStart
	if (ActorStart)
	{
		CharacterDisplayTransform = ActorStart->GetActorTransform();
	}
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CharacterDisplay = GetWorld()->SpawnActor<ACharacterDisplay>(CharacterDisplayClass,CharacterDisplayTransform,ActorSpawnParams);

	GetOwningPlayer()->SetViewTarget(CharacterDisplay);
}

void ULobbyWidget::UpdateCharacterDisplay(const FPlayerSelection& PlayerSelection)
{
	if (!PlayerSelection.GetCharacterDefinition()) return;

	CharacterDisplay->ConfigureWithCharacterDefinition(PlayerSelection.GetCharacterDefinition());

	AbilityListView->ClearListItems();
	const TMap<ECAbilityInputId, TSubclassOf<UGameplayAbility>>* Abilities = PlayerSelection.GetCharacterDefinition()->GetAbilities();
	if (Abilities)
	 {
		AbilityListView->ConfigureAbility(*Abilities);
	}
}


