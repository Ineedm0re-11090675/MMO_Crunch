#include "CAssetManager.h"

UCAssetManager& UCAssetManager::Get()
{
	//Singleton ：整个游戏进程里，UE 只维护一个全局的 UAssetManager 实例
	UCAssetManager* Singleton = Cast<UCAssetManager>(GEngine->AssetManager.Get());
	if (Singleton)
	{
		return *Singleton;
	}
	UE_LOG(LogLoad,Fatal,TEXT("Asset Manager Needs to be of the type CAssetsManager"))
	return *NewObject<UCAssetManager>();
}

void UCAssetManager::LoadShopItem(const FStreamableDelegate& LoadFinishCallback)
{
	LoadPrimaryAssetsWithType(UPA_ShopItem::GetPrimaryAssetType(),TArray<FName>(),FStreamableDelegate::CreateUObject(this,&UCAssetManager::ShopItemLoadFinished,LoadFinishCallback));
}

bool UCAssetManager::GetLoadedItem(TArray<const UPA_ShopItem*>& OutItems) const
{
	TArray<UObject*> LoadedItems;
	bool bLoaded = GetPrimaryAssetObjectList(UPA_ShopItem::GetPrimaryAssetType(),LoadedItems);
	if (bLoaded)
	{
		for (UObject* Item : LoadedItems)
		{
			OutItems.Add(Cast<UPA_ShopItem>(Item));
		}
	}
	return bLoaded;
}

const FItemCollection* UCAssetManager::GetCombinationForItems(const UPA_ShopItem* Item) const
{
	return CombinationMap.Find(Item);
}

const FItemCollection* UCAssetManager::GetIngredientsForItems(const UPA_ShopItem* Item) const
{ 
	return IngredientMap.Find(Item); 
}

void UCAssetManager::ShopItemLoadFinished(FStreamableDelegate Callback)
{
	Callback.ExecuteIfBound();
	BuildItemMap();
}

void UCAssetManager::BuildItemMap()
{
	TArray<const UPA_ShopItem*> LoadedItems;
	if (GetLoadedItem(LoadedItems))
	{
		for (const UPA_ShopItem* Item : LoadedItems)
		{
			if (Item->GetIngredientItems().Num() == 0)
			{
				continue;
			}
			TArray<const UPA_ShopItem*> Items;
			for (const TSoftObjectPtr<UPA_ShopItem>& Ingredient : Item->GetIngredientItems())
			{
				UPA_ShopItem* IngredientItem = Ingredient.LoadSynchronous();
				Items.Add(IngredientItem);
				AddToCombinationMap(IngredientItem, Item);
			}
			IngredientMap.Add(Item,Items);
		}
	}
}

void UCAssetManager::AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem)
{
	FItemCollection* Combinations = CombinationMap.Find(Ingredient);
	if (Combinations)
	{
		if (!Combinations->Contains(CombinationItem))
		{
			Combinations->AddItem(CombinationItem);
		}
	}else
	{
		CombinationMap.Add(Ingredient,FItemCollection{TArray<const UPA_ShopItem*>{CombinationItem}});
	}
}
