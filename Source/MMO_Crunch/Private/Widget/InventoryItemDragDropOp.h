#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryItemDragDropOp.generated.h"

class UInventoryItemWidget;
class UItemWidget;
UCLASS()
class UInventoryItemDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()
public:
	void SetDraggedItem(UInventoryItemWidget* DraggedItem);
private:
	UPROPERTY(EditDefaultsOnly,Category = "Visual")
	TSubclassOf<UItemWidget> DragVisualClass;
};
