#include "TeamSelectionWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"


void UTeamSelectionWidget::SetSlotId(uint8 NewSlotId)
{
	SlotID = NewSlotId;
}

void UTeamSelectionWidget::UpdateSlotInfo(const FString& PlayerNickName)
{
	InfoText->SetText(FText::FromString(PlayerNickName));
}

void UTeamSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SelectionButton->OnClicked.AddDynamic(this,&UTeamSelectionWidget::SelectionButtonClicked);
}

void UTeamSelectionWidget::SelectionButtonClicked()
{
	OnSlotlicked.Broadcast(SlotID);
}
