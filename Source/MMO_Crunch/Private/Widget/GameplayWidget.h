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

	void ToggleShop();
private:
	UPROPERTY(meta=(BindWidget))
	class UValueGauge* HealthBar;
	UPROPERTY(meta=(BindWidget))
	UValueGauge* ManaBar;

	UPROPERTY(meta=(BindWidget))
	class UAbilityListView* AbilityList;

	UPROPERTY(meta=(BindWidget))
	class UShopWidget* ShopWidget;
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
	UPROPERTY(Transient,meta=(BindWidgetAnim))
	UWidgetAnimation* ShopPopupAnim;
	void PlayShopPopupAnimation(bool bPlayForward);
	void SetOwningPawnInputEnbaled(bool bPawnInputEnabled);
	void SetShowMouseCursor(bool bShowMouseCursor);
	void SetFocusToGameAndUI();
	void SetFocusToGameOnly();
	UPROPERTY()
	class UAbilitySystemComponent* AbilitySystemComponent;
};
