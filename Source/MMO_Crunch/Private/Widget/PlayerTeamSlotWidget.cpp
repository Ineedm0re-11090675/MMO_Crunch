#include "PlayerTeamSlotWidget.h"

#include "Character/PA_CharacterDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPlayerTeamSlotWidget::UpdateSlot(const FString& PlayerName, const UPA_CharacterDefinition* CharacterDefinition)
{
	CachedPlayerNamePtr = PlayerName;
	if (CharacterDefinition)
	{
		PlayerCharacterIcon->GetDynamicMaterial()->SetTextureParameterValue(CharacterIconMatParaName,CharacterDefinition->LoadIcon());
		PlayerCharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(CharacterEmptyNameParaName,0.f);
		CachedCharacterNamePtr = CharacterDefinition->GetCharacterName();
	}
	else
	{
		PlayerCharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(CharacterEmptyNameParaName,1.f);
		CachedCharacterNamePtr = "";
	}
	UpdateNameText();
}

void UPlayerTeamSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerCharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(CharacterEmptyNameParaName,1.f);
	CachedCharacterNamePtr ="";
}

void UPlayerTeamSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	NameText->SetText(FText::FromString(CachedCharacterNamePtr));
	PlayAnimationForward(HoverAnim);
}

void UPlayerTeamSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	NameText->SetText(FText::FromString(CachedPlayerNamePtr));
	PlayAnimationReverse(HoverAnim);
}

void UPlayerTeamSlotWidget::UpdateNameText()
{
	if (IsHovered())
	{
		NameText->SetText(FText::FromString(CachedCharacterNamePtr));
	}
	else
	{
		NameText->SetText(FText::FromString(CachedPlayerNamePtr));
	}
}
