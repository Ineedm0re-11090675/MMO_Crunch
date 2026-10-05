#include "CharacterEntryWidget.h"

#include "Character/PA_CharacterDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCharacterEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	CharacterDefinition = Cast<UPA_CharacterDefinition>(ListItemObject);

	if (CharacterDefinition)
	{
		CharacterIcon->GetDynamicMaterial()->SetTextureParameterValue(IconTextureMatParaName,CharacterDefinition->LoadIcon());
		CharacterNameText->SetText(FText::FromString(CharacterDefinition->GetCharacterName()));
		
	}
}

void UCharacterEntryWidget::SetSelected(bool bIsSelected)
{
	CharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(SaturationMatParaName,bIsSelected? 0.f : 1.f);
}
