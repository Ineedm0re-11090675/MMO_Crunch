#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "InventoryItem.generated.h"

class UPA_ShopItem;
class UAbilitySystemComponent;

/*
 *Handle很有意思
 *之前一直说RPC对网络的要求很高
 *所以做成了Handle，一个int32类型的数据来作为标签，去标记InventoryItem
 *可以有效的减少网络传输的数据量
 */

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()
public:
	FInventoryItemHandle();
	static FInventoryItemHandle CreateHandle();
	static FInventoryItemHandle InvalidHandle();

	bool IsValid()const;
	uint32 GetHandleId()const {return HandleId;}
private:

	explicit FInventoryItemHandle(uint32 Id);
	UPROPERTY()
	uint32 HandleId;

	static uint32 GenerateNextId();
	static uint32 GetInvalidId();
};
bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& Key);

UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()
public:
	//return true if add successfully
	bool AddStackCount();

	//return true if stack not empty after reduce;
	bool ReduceStackCount();
	
	bool IsStackFull()const;

	bool SetStackCount(int NewStackCount);

	bool IsForItem(const UPA_ShopItem* ShopItem) const;
	UInventoryItem();
	bool IsValid() const;
	void InitItem(const FInventoryItemHandle& NewHandle,const UPA_ShopItem* NewShopItem);
	const UPA_ShopItem* GetShopItem() const {return ShopItem;}
	FInventoryItemHandle GetHandle() const {return Handle;}

	FORCEINLINE int GetStackCount() const {return StackCount;} 
	
	void ApplyGASModification(UAbilitySystemComponent* AbilitySystemComponent);
	bool TryActivateGrantedAbility(UAbilitySystemComponent* AbilitySystemComponent);
	void ApplyConsumeEffect(UAbilitySystemComponent* AbilitySystemComponent);
	void SetSlotNumber(int NewSlotNumber);
	void RemoveGASModification(UAbilitySystemComponent* AbilitySystemComponent);
private:
	UPROPERTY()
	const UPA_ShopItem* ShopItem;
	FInventoryItemHandle Handle;

	int StackCount;
	int SlotNumber;
	
	FActiveGameplayEffectHandle ApplyEquippedEffectHandle;
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;
};
