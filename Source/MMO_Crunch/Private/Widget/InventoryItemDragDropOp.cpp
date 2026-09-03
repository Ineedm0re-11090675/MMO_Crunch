#include "InventoryItemDragDropOp.h"
#include "Widget/InventoryItemWidget.h"

void UInventoryItemDragDropOp::SetDraggedItem(UInventoryItemWidget* DraggedItem)
{
	Payload = DraggedItem;
	//所作的就是一些拖拽视觉效果
	if (DragVisualClass)
	{
		UItemWidget* DragItemWidget = CreateWidget<UItemWidget>(GetWorld(), DragVisualClass);		
		if (DragItemWidget)
		{
			DragItemWidget->SetIcon(DraggedItem->GetIconTexture());
			DefaultDragVisual = DragItemWidget;
		}
	}
}
