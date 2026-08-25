 #include "ValueGauge.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "LandscapeDataAccess.h"

 void UValueGauge::NativeConstruct()
 {
	 Super::NativeConstruct();
 	ProgressBar->SetFillColorAndOpacity(BarColor);
 	ValueText->SetFont(ValueTextFont);

 	ValueText->SetVisibility(bValueTextVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
 	ProgressBar->SetVisibility(bProgressBarVisible? ESlateVisibility::Visible : ESlateVisibility::Hidden);
 	
 }

 void UValueGauge::SetValue(float NewValue, float NewMaxValue)
 {
 	MaxValueCache = NewMaxValue;
 	ValueCache = NewValue;
 	if (NewMaxValue == 0)
 	{
 		UE_LOG(LogTemp,Warning,TEXT("UValueGauge::SetValue -Max Value cant be zero"),*GetName());
 		return;
 	}
 	float Percent =NewValue / NewMaxValue;
 	ProgressBar->SetPercent(Percent);

 	/*
 	 *FNumberFormattingOptions 是 UE 专门用于控制数字转文本时格式的结构体。
 	 *在此制定的格式转换
 	 */
 	FNumberFormattingOptions NumberFormattingOptions =FNumberFormattingOptions().SetMaximumFractionalDigits(0);

 	ValueText->SetText(
 		FText::Format(
 			FTextFormat::FromString("{0}/{1}"),
 			FText::AsNumber(NewValue,&NumberFormattingOptions),
 			FText::AsNumber(NewMaxValue,&NumberFormattingOptions)
 			)
 		);
 }

 void UValueGauge::SetAndBoundToGameplayAttribute(class UAbilitySystemComponent* AbilitySystemComponent,
	 const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
 {
 	if (AbilitySystemComponent)
 	{
 		/*
 		 *Set
 		 */
 		bool bFound;
 		float Value =AbilitySystemComponent->GetGameplayAttributeValue(Attribute,bFound);
 		float MaxValue =AbilitySystemComponent->GetGameplayAttributeValue(MaxAttribute,bFound);
 		if (bFound)
 		{
 			SetValue(Value,MaxValue);
 		}

 		/*
 		 *Bound
 		 */
 		
 		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UValueGauge::ValueChanged);
 		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UValueGauge::MaxValueChanged);
 	}
 }

 void UValueGauge::ValueChanged(const FOnAttributeChangeData& ChangeData)
 {
 	SetValue(ChangeData.NewValue,MaxValueCache);
 }

 void UValueGauge::MaxValueChanged(const FOnAttributeChangeData& ChangedData)
 {
 	SetValue(ValueCache,ChangedData.NewValue);
 }
