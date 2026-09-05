#include "InventoryItem.h"

#include "AbilitySystemComponent.h"
#include "PA_ShopItem.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"

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

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem,UAbilitySystemComponent* AbilitySystemComponent)
{
	Handle = NewHandle;
	ShopItem = NewShopItem;
	OwnerAbilitySystemComponent = AbilitySystemComponent;
	if (OwnerAbilitySystemComponent)
	{
		OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this,&UInventoryItem::ManaUpdated);
	}
	ApplyGASModification();
}

void UInventoryItem::ApplyGASModification()
{
	if (!ShopItem || !OwnerAbilitySystemComponent)
	{
		return;
	}
	if (!OwnerAbilitySystemComponent->GetOwner() || !OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
	{
		return;
	}
	TSubclassOf<UGameplayEffect> EquippedEffect = GetShopItem()->GetEquippedEffect(); 
	if (EquippedEffect)
	{
		ApplyEquippedEffectHandle =OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquippedEffect,1,OwnerAbilitySystemComponent->MakeEffectContext());
	}
	TSubclassOf<UGameplayAbility> GrantedAbility =  GetShopItem()->GetGrantedAbility();
	if (GrantedAbility)
	{
		GrantedAbilitySpecHandle = OwnerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
	}
}

bool UInventoryItem::TryActivateGrantedAbility()
{
	if (!GrantedAbilitySpecHandle.IsValid())
	{
		return false;
	}
	if (OwnerAbilitySystemComponent && OwnerAbilitySystemComponent->TryActivateAbility(GrantedAbilitySpecHandle))
	{
		return true;
	}
	return false;
}

void UInventoryItem::ApplyConsumeEffect()
{
	if (!ShopItem) return;

	TSubclassOf<UGameplayEffect> ConsumeEffect = ShopItem->GetConsumeEffect();
	if (!ConsumeEffect) return;

	OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(ConsumeEffect,1,OwnerAbilitySystemComponent->MakeEffectContext());
}

void UInventoryItem::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}

void UInventoryItem::RemoveGASModification()
{
	if (!OwnerAbilitySystemComponent)
	{
		return;
	}

	//由于UE自动清理指针有一段真空期，这段时间内我们不希望蓝量更改被监听，所以我们手动Remove
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).RemoveAll(this);
	if (OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
	{
		//这种留下回执的形式很方便后续消除处理
		if (ApplyEquippedEffectHandle.IsValid())
		{
			OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(ApplyEquippedEffectHandle);
		}
		if (GrantedAbilitySpecHandle.IsValid())
		{
			OwnerAbilitySystemComponent->SetRemoveAbilityOnEnd(GrantedAbilitySpecHandle);
		}
	}

}

bool UInventoryItem::IsGrantingAbility(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (!ShopItem) return false;
	TSubclassOf<UGameplayAbility> GrantedAbility = ShopItem->GetGrantedAbility();
	return GrantedAbility == AbilityClass;
}

bool UInventoryItem::IsGrantingAnyAbility() const
{
	if (!ShopItem) return false;

	return ShopItem->GetGrantedAbility() != nullptr;
}

float UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
	return UCAbilitySystemStatics::GetCooldownRemainingFor(GetShopItem()->GetGrantedAbilityCOD(),*OwnerAbilitySystemComponent);
}

float UInventoryItem::GetAbilityCooldownTimeDuration() const
{
	return UCAbilitySystemStatics::GetCooldownDurationFor(GetShopItem()->GetGrantedAbilityCOD(),*OwnerAbilitySystemComponent,1); 
}

float UInventoryItem::GetAbilityManaCost() const
{
	return UCAbilitySystemStatics::GetManaCostFor(GetShopItem()->GetGrantedAbilityCOD(),*OwnerAbilitySystemComponent,1);
}

bool UInventoryItem::CanCastAbility() const
{
	if (!IsGrantingAnyAbility() || !OwnerAbilitySystemComponent)
		return false;

	FGameplayAbilitySpec* Spec = OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(GrantedAbilitySpecHandle);
	if (Spec)
	{
		return UCAbilitySystemStatics::CheckAbilityCost(*Spec, *OwnerAbilitySystemComponent);
	}

	return UCAbilitySystemStatics::CheckAbilityCost(FGameplayAbilitySpec(GetShopItem()->GetGrantedAbilityCOD()), *OwnerAbilitySystemComponent);
}

void UInventoryItem::ManaUpdated(const FOnAttributeChangeData& Data)
{
	OnAbilityCanCastUpdated.Broadcast(CanCastAbility());
}
