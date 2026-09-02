#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UCHeroAttributeSet;
class UAbilitySystemComponent;
class UPA_ShopItem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	void TryPurchase(const UPA_ShopItem* ItemToPurchase);
	float GetGold() const ;
private:
	UPROPERTY()
	UAbilitySystemComponent* OwnAbilitySystemComponent;
	/*
	*Server
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);
};
