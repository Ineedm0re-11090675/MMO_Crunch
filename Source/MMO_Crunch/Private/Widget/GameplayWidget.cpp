#include "GameplayWidget.h"

#include "AbilityListView.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
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


/*
 *当前路径
 *PlayerController生成GameplayWidget的同时 Configure，同时访问Character拿到Ability Map
 *触发ListView的Configure
 *ListView 的Configure在生成AbilityGauge之前 挂了委托 ，委托从DataTable里找到信息，更新了Ability的Icon
 */
