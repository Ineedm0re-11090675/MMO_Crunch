#include "AbilityListView.h"

#include "AbilityGauge.h"
#include "GAS/CGameplayAbility.h"
void UAbilityListView::ConfigureAbility(const TMap<ECAbilityInputId, TSubclassOf<UGameplayAbility>>& Abilities)
{
	OnEntryWidgetGenerated().AddUObject(this,&UAbilityListView::AbilityGaugeGenerated);
	//这个委托 挂着下面的AddItem
	for (const TPair<ECAbilityInputId,TSubclassOf<UGameplayAbility>>& Pair : Abilities)
	{
		AddItem(Pair.Value.GetDefaultObject());
	}
}

void UAbilityListView::AbilityGaugeGenerated(UUserWidget& Widget)
{
	UAbilityGauge* AbilityGauge = Cast<UAbilityGauge>(&Widget);
	if (AbilityGauge)
	{
		AbilityGauge->ConfigureAbilityWithWidgetData(FindWidgetDataForAbility(AbilityGauge->GetListItem<UGameplayAbility>()->GetClass()));
	}
}

const struct FAbilityWidgetData* UAbilityListView::FindWidgetDataForAbility(
	const TSubclassOf<UGameplayAbility>& AbilityClass) const
{

	/*
	 *根据AbilityClass去找对应的DataWidget，还能定位到Icon
	 */
	
	if (!AbilityTable) return nullptr;

	for (auto& AbilityWidgetTableData : AbilityTable->GetRowMap() )
	{
		const FAbilityWidgetData* WidgetData = AbilityTable->FindRow<FAbilityWidgetData>(AbilityWidgetTableData.Key,"");
		if (WidgetData->AbilityClass ==  AbilityClass)
		{
			return WidgetData;
		}
	}
	return nullptr;
}
