#include "InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PA_ShopItem.h"
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

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
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

void UInventoryComponent::Server_ActivateItem_Implementation(FInventoryItemHandle Handle)
{
	UInventoryItem* NewItem = GetInventoryItemFromHandle(Handle);
	if(!NewItem) return;

	NewItem->TryActivateGrantedAbility(OwnAbilitySystemComponent);
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
		UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
		FInventoryItemHandle NewHandle = FInventoryItemHandle::CreateHandle();
		InventoryItem->InitItem(NewHandle, NewItem);
		InventoryMap.Add(NewHandle, InventoryItem);
		OnInventoryAdded.Broadcast(InventoryItem);
		UE_LOG(LogTemp, Warning, TEXT("Grant Item %s , with Id %d"),
			   *(InventoryItem->GetShopItem()->GetItemName().ToString()), NewHandle.GetHandleId());
		Client_ItemAdded(NewHandle, NewItem);
		InventoryItem->ApplyGASModification(OwnAbilitySystemComponent);
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
	Item->ApplyConsumeEffect(OwnAbilitySystemComponent);
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
	Item->RemoveGASModification(OwnAbilitySystemComponent);
	OnItemRemoved.Broadcast(Item->GetHandle());
	InventoryMap.Remove(Item->GetHandle());
	Client_ItemRemoved(Item->GetHandle());
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
                                                          const UPA_ShopItem* NewItem)
{
	if (GetOwner()->HasAuthority())
	{
		return;
	}
	UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
	InventoryItem->InitItem(AssignedHandle, NewItem);
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
	if (IsFullFor(ItemToPurchase))
	{
		return;
	}
	OwnAbilitySystemComponent->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive,
	                                               -ItemToPurchase->GetPrice());

	GrantItem(ItemToPurchase);
}
