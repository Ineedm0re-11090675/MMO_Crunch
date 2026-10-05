#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityToolTip.generated.h"

UCLASS()
class UAbilityToolTip : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetAbilityInfo(const FName& AbilityName , UTexture2D* AbilityTexture , const FText& AbilityDescription , float AbilityCooldown ,float  AbilityCost);
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AbilityNameText;

	UPROPERTY(meta = (BindWidget))
	class UImage* AbilityIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AbilityDescriptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AbilityCooldownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AbilityCostText;
};
