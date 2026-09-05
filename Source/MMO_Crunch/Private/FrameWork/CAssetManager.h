#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Inventory/PA_ShopItem.h"
#include "CAssetManager.generated.h"

UCLASS()
class UCAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UCAssetManager& Get();
	void LoadShopItem(const FStreamableDelegate& LoadFinishCallback);
	bool GetLoadedItem(TArray<const UPA_ShopItem*>& OutItems) const;


	const FItemCollection* GetCombinationForItems(const UPA_ShopItem* Item) const;
	const FItemCollection* GetIngredientsForItems(const UPA_ShopItem* Item) const;
private:
	void ShopItemLoadFinished(FStreamableDelegate Callback);
	void BuildItemMap();
	void AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem);
	UPROPERTY()
	TMap<const UPA_ShopItem*, FItemCollection> CombinationMap;
	UPROPERTY()
	TMap<const UPA_ShopItem*, FItemCollection> IngredientMap;
};
