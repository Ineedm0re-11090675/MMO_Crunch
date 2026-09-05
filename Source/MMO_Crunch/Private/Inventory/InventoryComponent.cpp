#include "InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PA_ShopItem.h"
#include "FrameWork/CAssetManager.h"
#include "GAS/CHeroAttributeSet.h"

void UInventoryComponent::TryActivateItem(const FInventoryItemHandle& Handle)
{
	UInventoryItem* Item = GetInventoryItemFromHandle(Handle);
	if (!Item)
	{
		return;
	}
	Server_ActivateItem(Handle);
}

void UInventoryComponent::SellItem(const FInventoryItemHandle& Handle)
{
	Server_SellItem(Handle);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (OwnAbilitySystemComponent)
	{
		OwnAbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this,&UInventoryComponent::AbilityCommitted);
	}
}

void UInventoryComponent::TryPurchase(const UPA_ShopItem* ItemToPurchase)
{
	if (!OwnAbilitySystemComponent || !ItemToPurchase) return;
	Server_Purchase(ItemToPurchase);
}

float UInventoryComponent::GetGold() const
{
	bool bFound = false;
	if (OwnAbilitySystemComponent)
	{
		float Gold = OwnAbilitySystemComponent->GetGameplayAttributeValue(
			UCHeroAttributeSet::GetGoldAttribute(), bFound);
		if (bFound)
		{
			return Gold;
		}
	}
	return 0.f;
}

void UInventoryComponent::ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber)
{
	if (UInventoryItem* InventoryItem = GetInventoryItemFromHandle(Handle))
	{
		InventoryItem->SetSlotNumber(NewSlotNumber);
	}
}

UInventoryItem* UInventoryComponent::GetInventoryItemFromHandle(const FInventoryItemHandle& Handle)
{
	UInventoryItem* const* FoundItem = InventoryMap.Find(Handle);
	if (FoundItem)
	{
		return *FoundItem;
	}
	return nullptr;
}

bool UInventoryComponent::IsFullFor(const UPA_ShopItem* ShopItem) const
{
	if (!ShopItem) return false;
	if (IsAllSlotOccupied())
	{
		return GetAvailableStackForItem(ShopItem) == nullptr;
	}
	return false;
}

bool UInventoryComponent::IsAllSlotOccupied() const
{
	return InventoryMap.Num() >= GetCapacity();
}

UInventoryItem* UInventoryComponent::GetAvailableStackForItem(const UPA_ShopItem* Item) const
{
	if (!Item->IsStackable())
	{
		return nullptr;
	}
	for (const TPair<FInventoryItemHandle,UInventoryItem*>& ItemPair : InventoryMap)
	{
		if (ItemPair.Value && ItemPair.Value->IsForItem(Item) && !ItemPair.Value->IsStackFull())
		{
			return ItemPair.Value;
		}
	}
	return nullptr;
}

bool UInventoryComponent::FindIngredientsForItem(const UPA_ShopItem* Item, TArray<UInventoryItem*>& OutIngredient,
	const TArray<const UPA_ShopItem*>& IngredientToIgnore)
{
	const FItemCollection* Ingredients = UCAssetManager::Get().GetIngredientsForItems(Item);
	if (!Ingredients) return false;

	
	bool bAllFound = true;
	for (const UPA_ShopItem* Ingredient : Ingredients->GetItemArray())
	{
		if (IngredientToIgnore.Contains(Ingredient)) continue;
		UInventoryItem* FoundItem = TryGetItemForShopItem(Ingredient);
		if (!FoundItem)
		{
			bAllFound = false;
			break;
		}
		OutIngredient.Add(FoundItem);
	}
	return bAllFound;
}

UInventoryItem* UInventoryComponent::TryGetItemForShopItem(const UPA_ShopItem* Item) const
{
	if (!Item) return nullptr;
	for (const TPair<FInventoryItemHandle,UInventoryItem*>& ItemHandlePair : InventoryMap)
	{
		if (ItemHandlePair.Value && ItemHandlePair.Value->GetShopItem() == Item)
		{
			return ItemHandlePair.Value;
		}
	}
	return nullptr;
}

void UInventoryComponent::TryActivateItemInSlot(int SlotNumber)
{
	for (TPair<FInventoryItemHandle,UInventoryItem*>& ItemHandlePair : InventoryMap)
	{
		if (ItemHandlePair.Value->GetItemSlot() == SlotNumber)
		{
			Server_ActivateItem(ItemHandlePair.Key);
		}
	}
}

void UInventoryComponent::AbilityCommitted(UGameplayAbility* CommittedAbility)
{
	if (!CommittedAbility) return;

	float CooldownRemaining = 0.0f;
	float CooldownDuration = 0.0f;
	CommittedAbility->GetCooldownTimeRemainingAndDuration(
		CommittedAbility->GetCurrentAbilitySpecHandle(),
		CommittedAbility->GetCurrentActorInfo(),
		CooldownRemaining,
		CooldownDuration
		);
	for (TPair<FInventoryItemHandle,UInventoryItem*>& ItemHandlePair : InventoryMap)
	{
		if (!ItemHandlePair.Value) continue;

		if (ItemHandlePair.Value->IsGrantingAbility(CommittedAbility->GetClass()))
		{
			OnItemAbilityCommitted.Broadcast(ItemHandlePair.Key,CooldownRemaining,CooldownDuration);
		}
	}
}

void UInventoryComponent::Server_ActivateItem_Implementation(FInventoryItemHandle Handle)
{
	UInventoryItem* NewItem = GetInventoryItemFromHandle(Handle);
	if(!NewItem) return;

	NewItem->TryActivateGrantedAbility();
	const UPA_ShopItem* ShopItem = NewItem->GetShopItem();
	if (ShopItem->IsConsumable())
	{
		ConsumeItem(NewItem);
	}
}

bool UInventoryComponent::Server_ActivateItem_Validate(FInventoryItemHandle Handle)
{
	return true;
}

void UInventoryComponent::Server_SellItem_Implementation(FInventoryItemHandle Handle)
{
	UInventoryItem* NewItem = GetInventoryItemFromHandle(Handle);
	if(!NewItem || !NewItem->IsValid()) return;
	float SellPrice = NewItem->GetShopItem()->GetSellPrice();
	OwnAbilitySystemComponent->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(),EGameplayModOp::Additive,SellPrice * NewItem->GetStackCount());
	RemoveItem(NewItem);
}

bool UInventoryComponent::Server_SellItem_Validate(FInventoryItemHandle Handle)
{
	return true;
}

void UInventoryComponent::GrantItem(const UPA_ShopItem* NewItem)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	if (UInventoryItem* StackItem = GetAvailableStackForItem(NewItem))
	{
		StackItem->AddStackCount();
		OnItemStackCountChange.Broadcast(StackItem->GetHandle(),StackItem->GetStackCount());
		Client_ItemStackChanged(StackItem->GetHandle(),StackItem->GetStackCount());
	}
	else
	{
		if (TryItemCombination(NewItem))
		{
			return;
		}
		UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
		FInventoryItemHandle NewHandle = FInventoryItemHandle::CreateHandle();
		InventoryItem->InitItem(NewHandle,NewItem,OwnAbilitySystemComponent);
		InventoryMap.Add(NewHandle, InventoryItem);
		OnInventoryAdded.Broadcast(InventoryItem);
		UE_LOG(LogTemp, Warning, TEXT("Grant Item %s , with Id %d"),
			   *(InventoryItem->GetShopItem()->GetItemName().ToString()), NewHandle.GetHandleId());
		FGameplayAbilitySpecHandle GrantedAbilitySpecHandle = InventoryItem->GetGrantedAbilitySpecHandle();
		Client_ItemAdded(NewHandle, NewItem,GrantedAbilitySpecHandle);
	}
}

void UInventoryComponent::ConsumeItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	if (!Item)
		return;
	Item->ApplyConsumeEffect();
	if (!Item->ReduceStackCount())
	{
		RemoveItem(Item);
	}else
	{
		OnItemStackCountChange.Broadcast(Item->GetHandle(),Item->GetStackCount());
		//到client端广播
		Client_ItemStackChanged(Item->GetHandle(),Item->GetStackCount());
	}
}

void UInventoryComponent::RemoveItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	if (!Item)
		return;
	Item->RemoveGASModification();
	OnItemRemoved.Broadcast(Item->GetHandle());
	InventoryMap.Remove(Item->GetHandle());
	Client_ItemRemoved(Item->GetHandle());
}

bool UInventoryComponent::TryItemCombination(const UPA_ShopItem* Item)
{
	if (!GetOwner()->HasAuthority()) return false;

	if (!Item) return false;

	const FItemCollection* CombinationItems = UCAssetManager::Get().GetCombinationForItems(Item);
	if (!CombinationItems)return false;
	for (const UPA_ShopItem* CombinationItem : CombinationItems->GetItemArray())
	{
		TArray<UInventoryItem*> Ingredients;
		if (!FindIngredientsForItem(CombinationItem,Ingredients,TArray<const UPA_ShopItem*>{Item}))
		{
			continue;
		}

		for (UInventoryItem* IngredientItem : Ingredients)
		{
			RemoveItem(IngredientItem);
		}
		GrantItem(CombinationItem);
		return true; 
	}
	return false;
}



void UInventoryComponent::Client_ItemRemoved_Implementation(FInventoryItemHandle ItemHandle)
{
	if (GetOwner()->HasAuthority())
	{
		return;
	}

	UInventoryItem* FoundItem = GetInventoryItemFromHandle(ItemHandle);
	if (!FoundItem)
		return;
	//Item上的一些监听需要在指针销毁前remove，client端需要在此单独调用
	FoundItem->RemoveGASModification();
	OnItemRemoved.Broadcast(ItemHandle);
	InventoryMap.Remove(ItemHandle);
}	
 
void UInventoryComponent::Client_ItemStackChanged_Implementation(FInventoryItemHandle AssignedHandle, int NewCount)
{
	if (GetOwner()->HasAuthority())
	{
		return;
	}
	UInventoryItem* FoundItem = GetInventoryItemFromHandle(AssignedHandle);
	if (FoundItem)
	{
		FoundItem->SetStackCount(NewCount);
		OnItemStackCountChange.Broadcast(FoundItem->GetHandle(),FoundItem->GetStackCount());
	}
}

void UInventoryComponent::Client_ItemAdded_Implementation(FInventoryItemHandle AssignedHandle,
                                                          const UPA_ShopItem* NewItem,FGameplayAbilitySpecHandle GrantedAbilitySpecHandle)
{
	if (GetOwner()->HasAuthority())
	{
		return;
	}
	UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
	InventoryItem->InitItem(AssignedHandle, NewItem,OwnAbilitySystemComponent);
	InventoryItem->SetGrantedAbilitySpecHandle(GrantedAbilitySpecHandle);
	InventoryMap.Add(AssignedHandle, InventoryItem);
	OnInventoryAdded.Broadcast(InventoryItem);
	UE_LOG(LogTemp, Warning, TEXT("Grant Item %s , with Id %d"),
	       *(InventoryItem->GetShopItem()->GetItemName().ToString()), AssignedHandle.GetHandleId());
}

bool UInventoryComponent::Server_Purchase_Validate(const UPA_ShopItem* ItemToPurchase)
{
	return true;
}

void UInventoryComponent::Server_Purchase_Implementation(const UPA_ShopItem* ItemToPurchase)
{
	if (!ItemToPurchase) return;

	if (GetGold() < ItemToPurchase->GetPrice())
	{
		return;
	}
	if (!IsFullFor(ItemToPurchase))
	{
		OwnAbilitySystemComponent->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive,
												   -ItemToPurchase->GetPrice());
		GrantItem(ItemToPurchase);
		return;
	}
	if (TryItemCombination(ItemToPurchase))
	{
		OwnAbilitySystemComponent->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive,
												  -ItemToPurchase->GetPrice());
	}
}
