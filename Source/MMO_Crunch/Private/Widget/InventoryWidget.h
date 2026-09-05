#pragma once

#include "CoreMinimal.h"
 
#include "Blueprint/UserWidget.h"

#include "Inventory/InventoryItem.h"

#include "InventoryWidget.generated.h"

class UInventoryItem;
class UInventoryContextMenuWidget;
class UInventoryItemWidget;
UCLASS()
class UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
private:
	UPROPERTY(EditAnywhere,Category = "Inventory")
	TSubclassOf<UInventoryContextMenuWidget> ContextMenuWidgetClass;

	UPROPERTY()
	UInventoryContextMenuWidget* ContextMenuWidget;

	void SpawnContextMenuWidget();
	UFUNCTION()
	void SellFocusedItem();
	UFUNCTION()
	void UseFocusedItem();
	void ToggleContextMenu(const FInventoryItemHandle& Handle);
	void SetContextMenuVisible(bool bContextMenuVisible);
	void ClearContextMenu(); 
	FInventoryItemHandle CurrentFocusedItemHandle;
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
	void ItemAbilityCommitted(const FInventoryItemHandle& Handle,float CooldownRemaining,float CooldownDuration);

	UInventoryItemWidget* GetNextAvailableWidget()const;

	void HandleItemDragDrop(UInventoryItemWidget* DestinationWidget,UInventoryItemWidget* SourceWidget);
};
