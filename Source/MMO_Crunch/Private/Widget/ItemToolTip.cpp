#include "ItemToolTip.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/PA_ShopItem.h"

void UItemToolTip::SetItem(const UPA_ShopItem* Item)
{
	ItemTitleText->SetText(Item->GetItemName());
	ItemDescriptionText->SetText(Item->GetItemDescription());
	ItemPriceText->SetText(FText::AsNumber(Item->GetPrice()));
	IconImage->SetBrushFromTexture(Item->GetItemTexture());
	
}

void UItemToolTip::SetPrice(float NewPrice)
{
	ItemPriceText->SetText(FText::AsNumber((int)NewPrice));
}
