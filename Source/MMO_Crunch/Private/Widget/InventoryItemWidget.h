#pragma once

#include "CoreMinimal.h"
#include "ItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "InventoryItemWidget.generated.h"

class UInventoryItemWidget;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemDropped, UInventoryItemWidget* /*DestionationWidget*/, UInventoryItemWidget* /*SourceWidget*/);
class UInventoryItem;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnButtonClicked,const FInventoryItemHandle& );


UCLASS()
class UInventoryItemWidget  : public UItemWidget
{
	GENERATED_BODY()
public:
	FOnInventoryItemDropped OnInventoryItemDropped;
	FOnButtonClicked OnLeftClicked;
	FOnButtonClicked OnRightClicked;
	virtual void NativeConstruct() override;
	void UpdateInventoryItem(const UInventoryItem* Item);
	void SetSlotNumber(int NewSlotNumber);
	bool IsEmpty() const;
	void EmptySlot();
	FORCEINLINE int GetSlotNumber() const {return SlotNumber;}

	void UpdateStackCount();

	UTexture2D* GetIconTexture() const;

	FORCEINLINE const UInventoryItem* GetInventoryItem() const {return InventoryItem;}
	FInventoryItemHandle GetInventoryItemHandle() const;
private:
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	UTexture2D* EmptyTexture;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StackCountText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CooldownCountText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CooldownDurationText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ManaCostText;
 
	UPROPERTY()
	const UInventoryItem* InventoryItem;

	int SlotNumber;

	virtual void OnLeftButtonClicked() override;
	virtual void OnRightButtonClicked() override;
	/*
	 *Drag && Drop
	 */
private:
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditDefaultsOnly,Category="Drag Drop")
	TSubclassOf<class UInventoryItemDragDropOp> DragDropOpClass;
};
