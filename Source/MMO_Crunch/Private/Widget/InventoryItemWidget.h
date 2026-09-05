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
	void UpdateCanCastDisplay(bool bCanCast);
	
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

	/*
	*GAS
	*/
public:
	void StartCooldown(float CooldownRemaining,float CooldownDuration);
private:
	UPROPERTY(EditDefaultsOnly,Category="Cooldown")
	float CooldownUpdateInterval = 0.1f;

	void BindCanCastAbilityDelegate();
	void UnBindCanCastAbilityDelegate();
	void CooldownFinished();
	void UpdateCooldown();
	void ClearCooldown();

	FTimerHandle CooldownDurationTimerHandle;
	FTimerHandle CooldownUpdateTimerHandle;

	float CooldownTimeRemaining =0.f;
	float CooldownTimeDuration = 0.f;

	UPROPERTY(EditDefaultsOnly,Category="Cooldown")
	FName CooldownAmtDynamicMaterialParaName = "Percent";
	
	UPROPERTY(EditDefaultsOnly,Category="Cooldown")
	FName IconTextureDynamicMaterialParaName = "Icon";

	UPROPERTY(EditDefaultsOnly,Category="Cooldown")
	FName CanCastDynamicMaterialParaName = "CanCast";

	virtual void SetIcon(UTexture2D* Icon) override;
	FNumberFormattingOptions CooldownDisplayFormattingOptions;
};
