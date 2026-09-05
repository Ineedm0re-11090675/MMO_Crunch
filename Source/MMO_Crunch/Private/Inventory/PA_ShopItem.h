#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_ShopItem.generated.h"
class UPA_ShopItem;
USTRUCT(BlueprintType)
struct FItemCollection
{
	GENERATED_BODY()
public:
	FItemCollection();
	FItemCollection(const TArray<const UPA_ShopItem*>& InItems);
	void AddItem(const UPA_ShopItem* InItem,bool bAddUnique = false);
	bool Contains(const UPA_ShopItem* InItem) const;
	const TArray<const UPA_ShopItem*>& GetItemArray() const;
private:
	UPROPERTY()
	TArray<const UPA_ShopItem*> Items;
};




UCLASS()
class UPA_ShopItem : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetPrimaryAssetType();

	UTexture2D* GetItemTexture() const;
	FText GetItemDescription() const{return ItemDescription;}
	FText GetItemName() const{return ItemName;}
	float GetPrice() const {return Price;}
	float GetSellPrice() const {return Price/2;}

	TSubclassOf<class UGameplayEffect> GetEquippedEffect() const {return EquippedEffect;}
	TSubclassOf< UGameplayEffect> GetConsumeEffect() const{return ConsumeEffect;}
	TSubclassOf<class UGameplayAbility> GetGrantedAbility() const {return GrantedAbility;}

	bool IsStackable() const{return bIsStackable;}
	bool IsConsumable() const {return bIsConsumable;}
	int GetMaxStackAmt() const{return MaxStackAmt;}
	
	const TArray<TSoftObjectPtr<UPA_ShopItem>>& GetIngredientItems() const {return IngredientItems;}
	UGameplayAbility* GetGrantedAbilityCOD()const;
private:
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	float Price;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	FText ItemName;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	FText ItemDescription;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	bool bIsConsumable;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	TSubclassOf<UGameplayEffect> EquippedEffect;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	TSubclassOf<UGameplayEffect> ConsumeEffect;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	TSubclassOf<UGameplayAbility> GrantedAbility;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	bool bIsStackable =false;
	
	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	int MaxStackAmt = 3;

	UPROPERTY(EditDefaultsOnly,Category = "Shop Item")
	TArray<TSoftObjectPtr<UPA_ShopItem>> IngredientItems;
};
