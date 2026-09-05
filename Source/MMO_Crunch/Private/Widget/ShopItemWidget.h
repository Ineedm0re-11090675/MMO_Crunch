#pragma once

#include "CoreMinimal.h"
#include "ItemWidget.h"
#include "TreeNodeInterface.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ShopItemWidget.generated.h"

class UPA_ShopItem;
class UShopItemWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemPurchaseIssued,const UPA_ShopItem*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopItemSelected,const UShopItemWidget*);
UCLASS()

class UShopItemWidget : public UItemWidget , public IUserObjectListEntry , public ITreeNodeInterface
{
	GENERATED_BODY()
public:

	/*
	 *TreeNode Override
	 */

	//点击后，我们想单独生成一个合成树，所以我们复制一份而不是移动这一份
	virtual UUserWidget* GetWidget() const override;
	virtual TArray<const ITreeNodeInterface*> GetInputs() const override;
	virtual TArray<const ITreeNodeInterface*> GetOutputs() const override;
	virtual const UObject* GetItemObject() const override;
	
	FOnItemPurchaseIssued OnItemPurchaseIssued;
	FOnShopItemSelected OnShopItemClicked;
	FORCEINLINE const UPA_ShopItem* GetShopItem() const {return ShopItem;}
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
private:
	void CopyFromOther(const UShopItemWidget* Other);
	void InitWithShopItem(const UPA_ShopItem* NewShopItem);

	TArray<const ITreeNodeInterface*> ItemsToInterfaces(const TArray<const UPA_ShopItem*>& Items) const;
	
	UPROPERTY()
	const UPA_ShopItem* ShopItem;

	//可以从ListView里找Input
	UPROPERTY()
	const class UListView* ParentListView;
	
	virtual void OnLeftButtonClicked() override;
	virtual void OnRightButtonClicked() override;
};
