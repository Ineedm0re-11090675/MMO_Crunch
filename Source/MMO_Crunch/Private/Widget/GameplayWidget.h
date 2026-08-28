#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GameplayWidget.generated.h"
UCLASS()
class UGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void ConfigureAbility(const TMap<ECAbilityInputId,TSubclassOf<class UGameplayAbility>>& Abilities );
private:
	UPROPERTY(meta=(BindWidget))
	class UValueGauge* HealthBar;
	UPROPERTY(meta=(BindWidget))
	UValueGauge* ManaBar;

	UPROPERTY(meta=(BindWidget))
	class UAbilityListView* AbilityList;

	
	UPROPERTY(meta=(BindWidget))
	class UStatsGauge* AttackAttribute;

	UPROPERTY(meta=(BindWidget))
	UStatsGauge* Armor;

	UPROPERTY(meta=(BindWidget))
	UStatsGauge* MoveSpeed;

	UPROPERTY(meta=(BindWidget))
	UStatsGauge* Intelligence;

	UPROPERTY(meta=(BindWidget))
	UStatsGauge* Strength;
	
	UPROPERTY()
	class UAbilitySystemComponent* AbilitySystemComponent;
};
