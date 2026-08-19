#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadStatsGauge.generated.h"

UCLASS()
class UOverHeadStatsGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	void ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent);
private:
	UPROPERTY(meta=(BindWidget))
	class UValueGauge* HealthBar;
	UPROPERTY(meta=(BindWidget))
	UValueGauge* ManaBar;
};
