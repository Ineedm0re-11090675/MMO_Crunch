#include "PlayerTeamLayoutWidget.h"
#include "PlayerTeamSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Network/CNetStatic.h"
#include "Player/PlayerInfoTypes.h"

void UPlayerTeamLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();
	TeamOneLayoutBox->ClearChildren();
	TeamTwoLayoutBox->ClearChildren();
	if (!PlayerTeamSlotWidgetClass) return;

	for (int i = 0; i < UCNetStatic::GetPlayerCountPerTeam() * 2; ++i)
	{
		UPlayerTeamSlotWidget* PlayerTeamSlotWidget =CreateWidget<UPlayerTeamSlotWidget>(GetOwningPlayer(), PlayerTeamSlotWidgetClass);
		TeamSlotWidgets.Add(PlayerTeamSlotWidget);

		UHorizontalBoxSlot* NewSlot;
		if (i < UCNetStatic::GetPlayerCountPerTeam())
		{
			NewSlot = TeamOneLayoutBox->AddChildToHorizontalBox(PlayerTeamSlotWidget);
		}else
		{
			NewSlot = TeamTwoLayoutBox->AddChildToHorizontalBox(PlayerTeamSlotWidget);
		}
		NewSlot->SetPadding(PlayerTeamWidgetSlotMargin); 
	}
}

void UPlayerTeamLayoutWidget::UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UPlayerTeamSlotWidget* SlotWidget : TeamSlotWidgets)
	{
		SlotWidget->UpdateSlot("",nullptr);
	}

	for (const FPlayerSelection& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid()) continue;

		TeamSlotWidgets[PlayerSelection.GetSlot()]->UpdateSlot(PlayerSelection.GetPlayerNickName(),PlayerSelection.GetCharacterDefinition());
	}
}
