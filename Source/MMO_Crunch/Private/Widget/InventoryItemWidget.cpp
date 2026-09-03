#include "InventoryItemWidget.h"

#include "ItemToolTip.h"
#include "Components/TextBlock.h"
#include "Inventory/InventoryItem.h"
#include "Inventory/PA_ShopItem.h"
#include "Widget/InventoryItemDragDropOp.h"
void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EmptySlot();
}

void UInventoryItemWidget::UpdateInventoryItem(const UInventoryItem* Item)
{
	InventoryItem = Item;
	if (!InventoryItem || !InventoryItem->IsValid() || InventoryItem->GetStackCount() <=0)
	{
		EmptySlot();
		return;
	}
	SetIcon(Item->GetShopItem()->GetItemTexture());
	UItemToolTip* ToolTip = SetToolTipWidget(InventoryItem->GetShopItem());
	if (ToolTip)
	{
		ToolTip->SetPrice(InventoryItem->GetShopItem()->GetSellPrice());
	}
	if (InventoryItem->GetShopItem()->IsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);
		UpdateStackCount();
	}else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}

}

void UInventoryItemWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber =NewSlotNumber;
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !(InventoryItem->IsValid());
}

void UInventoryItemWidget::EmptySlot()
{
	InventoryItem = nullptr;
	SetIcon(EmptyTexture);
	SetToolTip(nullptr);

	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	ManaCostText->SetVisibility(ESlateVisibility::Hidden);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}

UTexture2D* UInventoryItemWidget::GetIconTexture() const
{
	if (InventoryItem && InventoryItem->GetShopItem())
	{
		return  InventoryItem->GetShopItem()->GetItemTexture();
	}
	return nullptr;
}

FInventoryItemHandle UInventoryItemWidget::GetInventoryItemHandle() const
{
	if (InventoryItem && InventoryItem->GetShopItem())
	{
		return InventoryItem->GetHandle(); 
	}
	return FInventoryItemHandle::InvalidHandle();
}

void UInventoryItemWidget::OnLeftButtonClicked()
{
	if (!IsEmpty())
	{
		OnLeftClicked.Broadcast(GetInventoryItemHandle());
	}
}

void UInventoryItemWidget::OnRightButtonClicked()
{
	if (!IsEmpty())
	{
		OnRightClicked.Broadcast(GetInventoryItemHandle());
	}
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!IsEmpty() && DragDropOpClass)
	{
		UInventoryItemDragDropOp* DragDropOp = NewObject<UInventoryItemDragDropOp>(this,DragDropOpClass);
		if (DragDropOp)
		{
			DragDropOp->SetDraggedItem(this);

			//输出操作是 = 拖拽
			OutOperation = DragDropOp;
		}
	}

}

bool UInventoryItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	//这个Payload被传入，是拖拽widget
	if (UInventoryItemWidget* OtherWidget = Cast<UInventoryItemWidget>(InOperation->Payload))
	{
		if (OtherWidget && !OtherWidget->IsEmpty())
		{
			OnInventoryItemDropped.Broadcast(this,OtherWidget);
			return true;
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
