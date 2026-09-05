#include "ShopItemWidget.h"

#include "IDetailTreeNode.h"
#include "Inventory/PA_ShopItem.h"
#include "Components/ListView.h"
#include "FrameWork//CAssetManager.h"
#include "FrameWork/CAssetManager.h"

UUserWidget* UShopItemWidget::GetWidget() const
{
	UShopItemWidget* Copy = CreateWidget<UShopItemWidget>(GetOwningPlayer(),GetClass());
	Copy->CopyFromOther(this);
	
	return Copy;
}

TArray<const ITreeNodeInterface*> UShopItemWidget::GetInputs() const
{
	const FItemCollection* Collection = UCAssetManager::Get().GetCombinationForItems(GetShopItem());
	if (Collection)
	{
		return ItemsToInterfaces(Collection->GetItemArray());
	}
	return TArray<const ITreeNodeInterface*>{};
}

TArray<const ITreeNodeInterface*> UShopItemWidget::GetOutputs() const
{
	const FItemCollection* Collection = UCAssetManager::Get().GetIngredientsForItems(GetShopItem());
	if (Collection)
	{
		return ItemsToInterfaces(Collection->GetItemArray());
	}
	return TArray<const ITreeNodeInterface*>{};
}

const UObject* UShopItemWidget::GetItemObject() const
{
	return ShopItem;
}

void UShopItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	InitWithShopItem(Cast<UPA_ShopItem>(ListItemObject));
	ParentListView = Cast<UListView>(IUserListEntry::GetOwningListView());
}

void UShopItemWidget::CopyFromOther(const UShopItemWidget* Other)
{
	OnItemPurchaseIssued = Other->OnItemPurchaseIssued;
	OnShopItemClicked = Other->OnShopItemClicked;
	ParentListView = Other->ParentListView;
	InitWithShopItem(Other->GetShopItem());
}

void UShopItemWidget::InitWithShopItem(const UPA_ShopItem* NewShopItem)
{
	ShopItem  = NewShopItem;
	if(!ShopItem)
	{
		return;
	}
	SetIcon(ShopItem->GetItemTexture());
	SetToolTipWidget(ShopItem);
}

TArray<const ITreeNodeInterface*> UShopItemWidget::ItemsToInterfaces(const TArray<const UPA_ShopItem*>& Items) const
{
	TArray<const ITreeNodeInterface*> ItemsToInterfaces;
	if (!ParentListView)
		return ItemsToInterfaces;
	for (const UPA_ShopItem* ShopItem : Items)
	{
		const UShopItemWidget* ItemWidget = ParentListView->GetEntryWidgetFromItem<UShopItemWidget>(ShopItem);
		if (ItemWidget)
		{
			ItemsToInterfaces.Add(ItemWidget);
		}
	}
	return ItemsToInterfaces;
}

void UShopItemWidget::OnLeftButtonClicked()
{
	OnShopItemClicked.Broadcast(this);
}

void UShopItemWidget::OnRightButtonClicked()
{
	OnItemPurchaseIssued.Broadcast(GetShopItem());
}
