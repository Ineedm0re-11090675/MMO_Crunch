#include "GameplayWidget.h"

#include "AbilityListView.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ShopWidget.h"
#include "ValueGauge.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GAS/CAttributeSet.h"
void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (AbilitySystemComponent)
	{
		/*
		 *给了一个ASC访问指针，然后给了两个地址方法让你去取，不是直接给你数据的
		 */
		HealthBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent,
			UCAttributeSet::GetHealthAttribute(),
			UCAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent,
			UCAttributeSet::GetManaAttribute(),
			UCAttributeSet::GetMaxManaAttribute());
			
	}
}

void UGameplayWidget::ConfigureAbility(
	const TMap<ECAbilityInputId, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	AbilityList->ConfigureAbility(Abilities);
}

void UGameplayWidget::ToggleShop()
{
	if (ShopWidget->GetVisibility() == ESlateVisibility::HitTestInvisible)
	{
		ShopWidget->SetVisibility(ESlateVisibility::Visible);
		PlayShopPopupAnimation(true);
		SetOwningPawnInputEnabled(false);
		SetShowMouseCursor(true);
		SetFocusToGameAndUI();
		ShopWidget->SetFocus();
	}else
	{
		ShopWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		PlayShopPopupAnimation(false);
		SetOwningPawnInputEnabled(true);
		SetShowMouseCursor(false);
		SetFocusToGameOnly();
	}
}

void UGameplayWidget::PlayShopPopupAnimation(bool bPlayForward)
{
	if (bPlayForward)
	{
		PlayAnimationForward(ShopPopupAnim);
	}else
	{
		PlayAnimationReverse(ShopPopupAnim);
	}
}

void UGameplayWidget::SetOwningPawnInputEnabled(bool bPawnInputEnabled)
{
	if (bPawnInputEnabled)
	{
		//关闭Player的，不关闭UI的
		GetOwningPlayerPawn()->EnableInput(GetOwningPlayer());
	}else
	{
		GetOwningPlayerPawn()->DisableInput(GetOwningPlayer());
	}
}

void UGameplayWidget::SetShowMouseCursor(bool bShowMouseCursor)
{
	GetOwningPlayer()->SetShowMouseCursor(bShowMouseCursor);
}

void UGameplayWidget::SetFocusToGameAndUI()
{
	FInputModeGameAndUI GameAndUIInputMode;
	//捕捉鼠标动作时 显示光标
	GameAndUIInputMode.SetHideCursorDuringCapture(false);
	GetOwningPlayer()->SetInputMode(GameAndUIInputMode);
}

void UGameplayWidget::SetFocusToGameOnly()
{
	FInputModeGameOnly GameOnlyInputMode;
	GetOwningPlayer()->SetInputMode(GameOnlyInputMode);
}


/*
 *当前路径
 *PlayerController生成GameplayWidget的同时 Configure，同时访问Character拿到Ability Map
 *触发ListView的Configure
 *ListView 的Configure在生成AbilityGauge之前 挂了委托 ，委托从DataTable里找到信息，更新了Ability的Icon
 */
