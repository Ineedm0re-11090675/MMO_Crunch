#include "GameplayWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ValueGauge.h"
#include "GAS/CAbilitySystemComponent.h"
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
