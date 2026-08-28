#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "StatsGauge.generated.h"

UCLASS()
class UStatsGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AttributeText;

	UPROPERTY(EditAnywhere,Category = "Visual")
	UTexture2D* IconTexture;

	UPROPERTY(EditAnywhere,Category = "Attribute")
	FGameplayAttribute Attribute;

	void SetValue(float NewVal);
	FNumberFormattingOptions NumberFormattingOptions;

	void OnValueChanged(const FOnAttributeChangeData& Data);
};
