#include "InventoryWidget.h"

#include "InventoryItemWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Inventory/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/InventoryContextMenuWidget.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		InventoryComponent =OwnerPawn->GetComponentByClass<UInventoryComponent>();
		if (InventoryComponent)
		{

			InventoryComponent->OnInventoryAdded.RemoveAll(this);
			InventoryComponent->OnItemStackCountChange.RemoveAll(this);
			InventoryComponent->OnItemRemoved.RemoveAll(this);
			InventoryComponent->OnItemAbilityCommitted.RemoveAll(this);
			
			InventoryComponent->OnInventoryAdded.AddUObject(this,&UInventoryWidget::ItemAdded);
			InventoryComponent->OnItemStackCountChange.AddUObject(this,&UInventoryWidget::ItemStackCountChanged);
			InventoryComponent->OnItemRemoved.AddUObject(this,&UInventoryWidget::ItemRemoved);
			InventoryComponent->OnItemAbilityCommitted.AddUObject(this,&UInventoryWidget::ItemAbilityCommitted);
			int Capacity = InventoryComponent->GetCapacity();

			ItemList->ClearChildren();
			// Codex建议：清理旧的槽位指针和 Handle 映射
			ItemWidgets.Empty();
			PopulatedItemEntryWidgets.Empty();
			
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
					NewEmptyWidget->OnRightClicked.AddUObject(this,&UInventoryWidget::ToggleContextMenu);
				}
			}
			SpawnContextMenuWidget();
		}
	}
}

void UInventoryWidget::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath,
	const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
	if (!NewWidgetPath.ContainsWidget(ContextMenuWidget->GetCachedWidget().Get()))
	{
		ClearContextMenu();
	}

}


void UInventoryWidget::SpawnContextMenuWidget()
{
	if (!ContextMenuWidgetClass) return;
	ContextMenuWidget = CreateWidget<UInventoryContextMenuWidget>(this,ContextMenuWidgetClass);
	if (ContextMenuWidget)
	{
		 ContextMenuWidget->GetSellButtonClickedEvent().AddDynamic(this,&UInventoryWidget::SellFocusedItem);
		ContextMenuWidget->GetUseButtonClickedEvent().AddDynamic(this,&UInventoryWidget::UseFocusedItem);
		ContextMenuWidget->AddToViewport(1);
		SetContextMenuVisible(false);
		
	}
}

void UInventoryWidget::SellFocusedItem()
{
	InventoryComponent->SellItem(CurrentFocusedItemHandle);
	SetContextMenuVisible(false);
}

void UInventoryWidget::UseFocusedItem()
{
	InventoryComponent->TryActivateItem(CurrentFocusedItemHandle);
	SetContextMenuVisible(false);
}

void UInventoryWidget::ToggleContextMenu(const FInventoryItemHandle& Handle)
{
	if (CurrentFocusedItemHandle == Handle)
	{
		ClearContextMenu();
	}
	CurrentFocusedItemHandle = Handle;
	UInventoryItemWidget** ItemWidgetPtrPtr =PopulatedItemEntryWidgets.Find(Handle);
	if (!ItemWidgetPtrPtr) return;
	UInventoryItemWidget* ItemWidget = *ItemWidgetPtrPtr;
	if (!ItemWidget) return;

	SetContextMenuVisible(true);
	FVector2D ItemAbsPos = ItemWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D{1.f,0.5f});

	FVector2D ItemWidgetPixelPos,ItemWidgetViewportPos;
	USlateBlueprintLibrary::AbsoluteToViewport(this,ItemAbsPos,ItemWidgetPixelPos,ItemWidgetViewportPos);


	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		int ViewportSizeX, ViewportSizeY;
		PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
		float Scale =UWidgetLayoutLibrary::GetViewportScale(this);

		int Overshoot = ItemWidgetPixelPos.Y + ContextMenuWidget->GetDesiredSize().Y * Scale - ViewportSizeY;
		if (Overshoot > 0)
		{
			ItemWidgetPixelPos.Y-=Overshoot;
		}
	}

	ContextMenuWidget->SetPositionInViewport(ItemWidgetPixelPos);
}

void UInventoryWidget::SetContextMenuVisible(bool bContextMenuVisible)
{
	if (ContextMenuWidget)
	{
		ContextMenuWidget->SetVisibility(bContextMenuVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden); 
	}
}

void UInventoryWidget::ClearContextMenu()
{
	ContextMenuWidget->SetVisibility(ESlateVisibility::Hidden);
	CurrentFocusedItemHandle = FInventoryItemHandle::InvalidHandle();
}

void UInventoryWidget::ItemAdded(const UInventoryItem* InventoryItem)
{
	if (!InventoryItem)
	{
		return;
	}
	// Codex建议：同一个物品 Handle 不允许重复占用库存槽位
	if (PopulatedItemEntryWidgets.Contains(InventoryItem->GetHandle()))
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

void UInventoryWidget::ItemAbilityCommitted(const FInventoryItemHandle& Handle, float CooldownRemaining,
	float CooldownDuration)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(Handle);
	if (FoundWidget)
	{
		(*FoundWidget)->StartCooldown(CooldownRemaining,CooldownDuration);
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
