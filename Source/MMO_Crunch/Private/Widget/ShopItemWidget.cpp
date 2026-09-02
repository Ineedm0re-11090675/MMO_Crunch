#include "ShopItemWidget.h"
#include "Inventory/PA_ShopItem.h"
void UShopItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	ShopItem  = Cast<UPA_ShopItem>(ListItemObject);
    if(!ShopItem)
    {
	    return;
    }
	SetIcon(ShopItem->GetItemTexture());
	SetToolTipWidget(ShopItem);
}

void UShopItemWidget::OnLeftButtonClicked()
{
	OnShopItemClicked.Broadcast(this);
}

void UShopItemWidget::OnRightButtonClicked()
{
	OnItemPurchaseIssued.Broadcast(GetShopItem());
}
