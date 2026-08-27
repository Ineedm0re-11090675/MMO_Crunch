#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "AbilityListView.generated.h"

UCLASS()
class UAbilityListView : public UListView
{
	GENERATED_BODY()
public:
	void ConfigureAbility(const TMap<ECAbilityInputId,TSubclassOf<class UGameplayAbility>>& Abilities );
private:
	UPROPERTY(EditAnywhere,Category = "Data")
	UDataTable* AbilityTable;

	const struct FAbilityWidgetData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

	void AbilityGaugeGenerated(UUserWidget& Widget);
};
