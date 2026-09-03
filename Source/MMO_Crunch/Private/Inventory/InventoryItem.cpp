#include "InventoryItem.h"

#include "AbilitySystemComponent.h"
#include "PA_ShopItem.h"

FInventoryItemHandle::FInventoryItemHandle()
	:HandleId{GetInvalidId()}
{
}

FInventoryItemHandle::FInventoryItemHandle(uint32 Id)
	:HandleId{Id}
{
}

FInventoryItemHandle FInventoryItemHandle::CreateHandle()
{
	return FInventoryItemHandle(GenerateNextId()); 
}

FInventoryItemHandle FInventoryItemHandle::InvalidHandle()
{
	static FInventoryItemHandle InvalidHandle = FInventoryItemHandle();
	return InvalidHandle;
}

bool FInventoryItemHandle::IsValid() const
{
	return HandleId != GetInvalidId();
}


uint32 FInventoryItemHandle::GenerateNextId()
{
	static uint32 StaticId = 1;
	return StaticId++;
}

uint32 FInventoryItemHandle::GetInvalidId()
{
	return 0;
}

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
	return Lhs.GetHandleId() == Rhs.GetHandleId();
}

uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
	return Key.GetHandleId();
}

bool UInventoryItem::AddStackCount()
{
	if (IsStackFull()) return false;
	++StackCount;
	return true;
}

bool UInventoryItem::ReduceStackCount()
{
	--StackCount;
	if (StackCount <=0) return false;
	return true;
}

bool UInventoryItem::IsStackFull() const
{
	return StackCount >= GetShopItem()->GetMaxStackAmt();
}

bool UInventoryItem::SetStackCount(int NewStackCount)
{
	if (NewStackCount > 0 && NewStackCount <= GetShopItem()->GetMaxStackAmt())
	{
		StackCount = NewStackCount;
 		return true;
	}
	return false;
}

bool UInventoryItem::IsForItem(const UPA_ShopItem* ShopItem) const
{
	if (!ShopItem) return false;

	return GetShopItem() == ShopItem; 
}

UInventoryItem::UInventoryItem()
	:StackCount{1}
{
}

bool UInventoryItem::IsValid() const
{
	return ShopItem != nullptr;
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem)
{
	Handle = NewHandle;
	ShopItem = NewShopItem;
}

void UInventoryItem::ApplyGASModification(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!ShopItem || !AbilitySystemComponent)
	{
		return;
	}
	if (!AbilitySystemComponent->GetOwner() || !AbilitySystemComponent->GetOwner()->HasAuthority())
	{
		return;
	}
	TSubclassOf<UGameplayEffect> EquippedEffect = GetShopItem()->GetEquippedEffect(); 
	if (EquippedEffect)
	{
		ApplyEquippedEffectHandle =AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquippedEffect,1,AbilitySystemComponent->MakeEffectContext());
	}
	TSubclassOf<UGameplayAbility> GrantedAbility =  GetShopItem()->GetGrantedAbility();
	if (GrantedAbility)
	{
		const FGameplayAbilitySpec* FoundSpec = AbilitySystemComponent->FindAbilitySpecFromClass(GrantedAbility);
		if (FoundSpec)
		{
			GrantedAbilitySpecHandle = FoundSpec->Handle;
		}else
		{
			GrantedAbilitySpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
		}
	}
}

bool UInventoryItem::TryActivateGrantedAbility(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!GrantedAbilitySpecHandle.IsValid())
	{
		return false;
	}
	if (AbilitySystemComponent && AbilitySystemComponent->TryActivateAbility(GrantedAbilitySpecHandle))
	{
		return true;
	}
	return false;
}

void UInventoryItem::ApplyConsumeEffect(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!ShopItem) return;

	TSubclassOf<UGameplayEffect> ConsumeEffect = ShopItem->GetConsumeEffect();
	if (!ConsumeEffect) return;

	AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(ConsumeEffect,1,AbilitySystemComponent->MakeEffectContext());
}

void UInventoryItem::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}

void UInventoryItem::RemoveGASModification(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return;
	}
	//这种留下回执的形式很方便后续消除处理
	if (ApplyEquippedEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(ApplyEquippedEffectHandle);
	}
	if (GrantedAbilitySpecHandle.IsValid())
	{
		AbilitySystemComponent->SetRemoveAbilityOnEnd(GrantedAbilitySpecHandle);
	}
}
