#include "ItemWidget.h"
#include "ItemToolTip.h"
#include "Components/Image.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

void UItemWidget::SetIcon(UTexture2D* Icon)
{
	ItemIcon->SetBrushFromTexture(Icon);
}

UItemToolTip* UItemWidget::SetToolTipWidget(const UPA_ShopItem* Item)
{
	if (!Item) return nullptr;
	
	if (GetOwningPlayer() && ItemToolTipClass)
	{
		UItemToolTip* ToolTip = CreateWidget<UItemToolTip>(GetOwningPlayer(), ItemToolTipClass);
		if (ToolTip)
		{
			ToolTip->SetItem(Item);
			SetToolTip(ToolTip);
		}
		return ToolTip;
	}
	return nullptr;
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) 
	{
		/*
		*这个事件由当前 Widget 处理。
		当前 Widget 获得焦点。
		Handle去抢，它相当于告诉 UE：这个鼠标事件我已经处理了，不需要继续向父 Widget 传播。
		*/
		return FReply::Handled().SetUserFocus(TakeWidget());
	}
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget()).DetectDrag(TakeWidget(),EKeys::LeftMouseButton);
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply SuperReply= Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (HasAnyUserFocus())
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			OnLeftButtonClicked();
			return FReply::Handled();
		}
		if (InMouseEvent.GetEffectingButton() ==EKeys::RightMouseButton)
		{
			OnRightButtonClicked();
			return FReply::Handled();
		}
	}
	return SuperReply;
}

void UItemWidget::OnLeftButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("UItemWidget::OnLeftButtonClicked()")); 
}

void UItemWidget::OnRightButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("UItemWidget::OnRightButtonClicked()"));
}
