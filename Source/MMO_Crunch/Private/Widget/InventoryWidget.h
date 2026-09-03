#pragma once

#include "CoreMinimal.h"
 
#include "Blueprint/UserWidget.h"

#include "Inventory/InventoryItem.h"

#include "InventoryWidget.generated.h"

class UInventoryItem;

class UInventoryItemWidget;
UCLASS()
class UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta=(BindWidget))
	class UWrapBox* ItemList;
	
	UPROPERTY(EditDefaultsOnly,Category="Inventory")
	TSubclassOf<UInventoryItemWidget> ItemWidgetClass;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	UPROPERTY()
	TArray<UInventoryItemWidget*> ItemWidgets; 

	UPROPERTY()
	TMap<FInventoryItemHandle,UInventoryItemWidget*> PopulatedItemEntryWidgets;

	void ItemAdded(const UInventoryItem* InventoryItem);
	void ItemStackCountChanged(const FInventoryItemHandle& Handle, int NewCount);
	void ItemRemoved(const FInventoryItemHandle& Handle);

	UInventoryItemWidget* GetNextAvailableWidget()const;

	void HandleItemDragDrop(UInventoryItemWidget* DestinationWidget,UInventoryItemWidget* SourceWidget);
};
