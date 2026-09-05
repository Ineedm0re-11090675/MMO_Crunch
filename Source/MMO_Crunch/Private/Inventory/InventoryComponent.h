#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h"
#include "InventoryComponent.generated.h"


class UAbilitySystemComponent;
class UPA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryAddedDelegate,const UInventoryItem* /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate,const FInventoryItemHandle&,int /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemovedDelegate,const FInventoryItemHandle&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAbilityCommitted, const FInventoryItemHandle& ,float,float);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	FOnInventoryAddedDelegate OnInventoryAdded;
	FOnItemRemovedDelegate OnItemRemoved;
	FOnItemStackCountChangeDelegate OnItemStackCountChange;
	FOnItemAbilityCommitted OnItemAbilityCommitted;
	
	void TryActivateItem(const FInventoryItemHandle& Handle);
	void SellItem(const FInventoryItemHandle& Handle);
	virtual void BeginPlay() override;

	void TryPurchase(const UPA_ShopItem* ItemToPurchase);
	float GetGold() const ;
	FORCEINLINE int GetCapacity() const { return (Capacity); }

	void ItemSlotChanged(const FInventoryItemHandle& Handle,int NewSlotNumber);
	UInventoryItem* GetInventoryItemFromHandle(const FInventoryItemHandle& Handle);

	bool IsFullFor(const UPA_ShopItem* ShopItem)const ;
	
	bool IsAllSlotOccupied()const;
	UInventoryItem* GetAvailableStackForItem(const UPA_ShopItem* Item)const;
	bool FindIngredientsForItem(const UPA_ShopItem* Item,TArray<UInventoryItem*>& OutIngredient,const TArray<const UPA_ShopItem*>& IngredientToIgnore = TArray<const UPA_ShopItem*>{});

	UInventoryItem* TryGetItemForShopItem(const UPA_ShopItem* Item) const;


	void TryActivateItemInSlot(int SlotNumber);
private:
	void AbilityCommitted(UGameplayAbility* CommittedAbility);
	UPROPERTY(EditDefaultsOnly,Category = "Inventory")
	int Capacity = 6;
	UPROPERTY()
	UAbilitySystemComponent* OwnAbilitySystemComponent;

	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;
	/*
	*Server
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(FInventoryItemHandle Handle);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SellItem(FInventoryItemHandle Handle);

	void GrantItem(const UPA_ShopItem* NewItem);
	void ConsumeItem(UInventoryItem* Item);
	void RemoveItem(UInventoryItem* Item);

	bool TryItemCombination(const UPA_ShopItem* Item);
	/*
	 *Client
	 */
private:
	UFUNCTION(Client, Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignedHandle,const UPA_ShopItem* NewItem,FGameplayAbilitySpecHandle GrantedAbilitySpecHandle);

	UFUNCTION(Client, Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);
	UFUNCTION(Client, Reliable)
	void Client_ItemStackChanged(FInventoryItemHandle AssignedHandle,int NewCount);
};
