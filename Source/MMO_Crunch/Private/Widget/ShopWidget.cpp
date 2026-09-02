#include "ShopWidget.h"

#include "ShopItemWidget.h"
#include "Components/TileView.h"
#include "FrameWork/CAssetManager.h"
#include "Inventory/InventoryComponent.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	LoadShopItems();
	ShopItemList->OnEntryWidgetGenerated().AddUObject(this,&UShopWidget::ShopItemWidgetGenerated);
	if (APawn* OwnerPawn =GetOwningPlayerPawn() )
	{
		OwnerInventoryComponent = OwnerPawn->GetComponentByClass<UInventoryComponent>();
	}
}

void UShopWidget::LoadShopItems()
{
	//先让AssetManager加载
	UCAssetManager::Get().LoadShopItem(FStreamableDelegate::CreateUObject(this,&UShopWidget::ShopItemLoadFinished));
}

void UShopWidget::ShopItemLoadFinished()
{
	TArray<const UPA_ShopItem*> ShopItems;
	UCAssetManager::Get().GetLoadedItem(ShopItems);
	for (const UPA_ShopItem* ShopItem : ShopItems)
	{
		ShopItemList->AddItem(const_cast<UPA_ShopItem*>(ShopItem));
	}
}

void UShopWidget::ShopItemWidgetGenerated(UUserWidget& NewWidget)
{
	//再把加载后的商品放入list
	UShopItemWidget* ItemWidget = Cast<UShopItemWidget>(&NewWidget);
	if (ItemWidget)
	{
		if (OwnerInventoryComponent)
		{
			ItemWidget->OnItemPurchaseIssued.AddUObject(OwnerInventoryComponent,&UInventoryComponent::TryPurchase);
		}
		ItemsMap.Add(ItemWidget->GetShopItem(),ItemWidget);
	}
}
