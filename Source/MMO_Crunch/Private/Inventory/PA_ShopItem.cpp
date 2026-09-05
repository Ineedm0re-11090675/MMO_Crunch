#include "PA_ShopItem.h"
#include "Abilities/GameplayAbility.h"
FItemCollection::FItemCollection()
	:Items{}
{
}

FItemCollection::FItemCollection(const TArray<const UPA_ShopItem*>& InItems)
	:Items(InItems)
{
}

void FItemCollection::AddItem(const UPA_ShopItem* InItem, bool bAddUnique)
{
	if (bAddUnique && Items.Contains(InItem))
	{
		return;
	}
	Items.Add(InItem);
}

bool FItemCollection::Contains(const UPA_ShopItem* InItem) const
{
	return Items.Contains(InItem);
}

const TArray<const UPA_ShopItem*>& FItemCollection::GetItemArray() const
{
	return Items;
}

FPrimaryAssetId UPA_ShopItem::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetPrimaryAssetType(),GetFName());
}

FPrimaryAssetType UPA_ShopItem::GetPrimaryAssetType()
{
	return FPrimaryAssetType("ShopItem");
}

UTexture2D* UPA_ShopItem::GetItemTexture() const
{
	return Icon.LoadSynchronous();
}

UGameplayAbility* UPA_ShopItem::GetGrantedAbilityCOD() const
{
	if (GrantedAbility)
	{
		return Cast<UGameplayAbility>(GrantedAbility->GetDefaultObject());
	}

	return nullptr;
}
