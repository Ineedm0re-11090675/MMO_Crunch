#include "InventoryWidget.h"

#include "InventoryItemWidget.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Inventory/InventoryComponent.h"


void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		InventoryComponent =OwnerPawn->GetComponentByClass<UInventoryComponent>();
		if (InventoryComponent)
		{
			InventoryComponent->OnInventoryAdded.AddUObject(this,&UInventoryWidget::ItemAdded);
			InventoryComponent->OnItemStackCountChange.AddUObject(this,&UInventoryWidget::ItemStackCountChanged);
			InventoryComponent->OnItemRemoved.AddUObject(this,&UInventoryWidget::ItemRemoved);
			int Capacity = InventoryComponent->GetCapacity();

			ItemList->ClearChildren();

			for (int i = 0; i < Capacity; ++i)
			{
				UInventoryItemWidget* NewEmptyWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(),ItemWidgetClass);
				if (NewEmptyWidget)
				{
					NewEmptyWidget->SetSlotNumber(i);
					UWrapBoxSlot* NewItemSlot = ItemList->AddChildToWrapBox(NewEmptyWidget);
					NewItemSlot->SetPadding(FMargin(2.0f));
					ItemWidgets.Add(NewEmptyWidget);

					NewEmptyWidget->OnInventoryItemDropped.AddUObject(this,&UInventoryWidget::HandleItemDragDrop);
					NewEmptyWidget->OnLeftClicked.AddUObject(InventoryComponent,&UInventoryComponent::TryActivateItem);
					
				}
			}
		}
	}
}

void UInventoryWidget::ItemAdded(const UInventoryItem* InventoryItem)
{
	if (!InventoryItem)
	{
		return;
	}
	if (UInventoryItemWidget* NextAvailableSlot = GetNextAvailableWidget())
	{
		NextAvailableSlot->UpdateInventoryItem(InventoryItem);
		PopulatedItemEntryWidgets.Add(InventoryItem->GetHandle(),NextAvailableSlot);
		if (InventoryComponent)
		{
			//从无到有也是一种change
			InventoryComponent->ItemSlotChanged(InventoryItem->GetHandle(),NextAvailableSlot->GetSlotNumber());
		}
	}
}

void UInventoryWidget::ItemStackCountChanged(const FInventoryItemHandle& Handle, int NewCount)
{
	UInventoryItemWidget** FoundWidget =PopulatedItemEntryWidgets.Find(Handle);
	if (FoundWidget)
	{
		//不需要传入Count，因为在Client端先改了Item里的Count，再广播的
		(*FoundWidget)->UpdateStackCount();
	}
}

void UInventoryWidget::ItemRemoved(const FInventoryItemHandle& Handle)
{  
	UInventoryItemWidget** FoundWidget =PopulatedItemEntryWidgets.Find(Handle);
	if (FoundWidget && *FoundWidget)
	{
		(*FoundWidget)->EmptySlot();
		PopulatedItemEntryWidgets.Remove(Handle);
	}
}

UInventoryItemWidget* UInventoryWidget::GetNextAvailableWidget() const
{
	for (UInventoryItemWidget* Widget :ItemWidgets)
	{
		if (Widget->IsEmpty())
		{
			return Widget;
		}
	}
	return nullptr;
}

void UInventoryWidget::HandleItemDragDrop(UInventoryItemWidget* DestinationWidget, UInventoryItemWidget* SourceWidget)
{
	const UInventoryItem* SrcItem =SourceWidget->GetInventoryItem();
	const UInventoryItem* DestItem =DestinationWidget->GetInventoryItem();

	DestinationWidget->UpdateInventoryItem(SrcItem);
	SourceWidget->UpdateInventoryItem(DestItem);

	//这里已经交换了，所以Widget不变的情况下，Item的Handle已经不一样了
	PopulatedItemEntryWidgets[DestinationWidget->GetInventoryItemHandle()] = DestinationWidget;
	if (InventoryComponent)
	{
		InventoryComponent->ItemSlotChanged(DestinationWidget->GetInventoryItemHandle(),DestinationWidget->GetSlotNumber());
	}
	//有可能拖到空格，nullptr取不出handle会崩溃，也不用担心map和component问题，因为本身就是空的 
	if (!SourceWidget->IsEmpty())
	{
		PopulatedItemEntryWidgets[SourceWidget->GetInventoryItemHandle()] = SourceWidget;
		if (InventoryComponent)
		{
			InventoryComponent->ItemSlotChanged(SourceWidget->GetInventoryItemHandle(),SourceWidget->GetSlotNumber());
		}
	}
}
