#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

class UTileView;
class UPA_ShopItem;
class UShopItemWidget;
class UInventoryComponent;
UCLASS()
class UShopWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta = (BindWidget))
	UTileView* ShopItemList;

	void LoadShopItems();
	void ShopItemLoadFinished();
	void ShopItemWidgetGenerated(UUserWidget& NewWidget);

	UPROPERTY()
	TMap<const UPA_ShopItem*,const UShopItemWidget*> ItemsMap;

	UPROPERTY()
	UInventoryComponent* OwnerInventoryComponent;
};
