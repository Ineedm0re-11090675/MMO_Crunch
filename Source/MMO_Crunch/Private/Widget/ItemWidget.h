#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"


class UItemToolTip;
class UPA_ShopItem;
UCLASS()
class UItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void SetIcon(UTexture2D* Icon);
protected:
	UItemToolTip* SetToolTipWidget(const UPA_ShopItem* Item);
	class UImage* GetItemIcon() const {return ItemIcon;};
private:
	UPROPERTY(meta = (BindWidget))
 	UImage* ItemIcon;
 
	UPROPERTY(EditDefaultsOnly,Category="ToolTip")
	TSubclassOf<UItemToolTip> ItemToolTipClass; 
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void OnLeftButtonClicked();
	virtual void OnRightButtonClicked();
};
