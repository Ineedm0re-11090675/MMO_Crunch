#include "InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PA_ShopItem.h"
#include "GAS/CHeroAttributeSet.h"

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
	bool bFound =false;
	if (OwnAbilitySystemComponent)
	{
		float Gold =OwnAbilitySystemComponent->GetGameplayAttributeValue(UCHeroAttributeSet::GetGoldAttribute(), bFound);
		if (bFound)
		{
			return Gold;
		}
	}
	return 0.f;
}

bool UInventoryComponent::Server_Purchase_Validate(const UPA_ShopItem* ItemToPurchase)
{
	return true;
}

void UInventoryComponent::Server_Purchase_Implementation(const UPA_ShopItem* ItemToPurchase)
{
	if (!ItemToPurchase) return;

	if (GetGold() < ItemToPurchase-> GetPrice())
	{
		return;
	}
	OwnAbilitySystemComponent->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(),EGameplayModOp::Additive,-ItemToPurchase->GetPrice());

	UE_LOG(LogTemp, Warning, TEXT("Purchase Item %s"),*(ItemToPurchase->GetName()));
}
