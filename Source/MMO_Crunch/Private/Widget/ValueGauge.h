#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "ValueGauge.generated.h"

struct FGameplayAttribute;

UCLASS()
class UValueGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void SetValue(float NewValue , float NewMaxValue);
	void SetAndBoundToGameplayAttribute(class UAbilitySystemComponent* AbilitySystemComponent,const FGameplayAttribute& Attribute,const FGameplayAttribute& MaxAttribute);
private:
	void ValueChanged(const FOnAttributeChangeData& ChangeData);
	void MaxValueChanged(const FOnAttributeChangeData& ChangedData);
	
	UPROPERTY(EditAnywhere,Category = "Visual")
	FSlateFontInfo ValueTextFont;

	UPROPERTY(EditAnywhere,Category = "Visual")
	bool bValueTextVisible =true;

	UPROPERTY(EditAnywhere,Category = "Visual")
	bool bProgressBarVisible=true;
	
	
	UPROPERTY(EditAnywhere,Category = "Visual")
	FLinearColor BarColor;
	UPROPERTY(VisibleAnywhere,meta = (BindWidget))
	class UProgressBar* ProgressBar;
	UPROPERTY(VisibleAnywhere,meta = (BindWidget))
	class UTextBlock* ValueText;

	float MaxValueCache;
	float ValueCache;
};
