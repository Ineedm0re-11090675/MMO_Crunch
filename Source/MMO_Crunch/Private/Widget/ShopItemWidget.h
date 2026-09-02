#pragma once

#include "CoreMinimal.h"
#include "ItemWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ShopItemWidget.generated.h"

class UPA_ShopItem;
class UShopItemWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemPurchaseIssued,const UPA_ShopItem*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopItemSelected,const UShopItemWidget*);
UCLASS()

class UShopItemWidget : public UItemWidget , public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	FOnItemPurchaseIssued OnItemPurchaseIssued;
	FOnShopItemSelected OnShopItemClicked;
	FORCEINLINE const UPA_ShopItem* GetShopItem() const {return ShopItem;}
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
private:
	UPROPERTY()
	const UPA_ShopItem* ShopItem;

	virtual void OnLeftButtonClicked() override;
	virtual void OnRightButtonClicked() override;
};
