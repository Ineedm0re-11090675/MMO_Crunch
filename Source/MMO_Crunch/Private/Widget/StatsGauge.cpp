#include "StatsGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void UStatsGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	Icon->SetBrushFromTexture(IconTexture);
}

void UStatsGauge::SetValue(float NewVal)
{
	AttributeText->SetText(FText::AsNumber(NewVal,&NumberFormattingOptions));
}

void UStatsGauge::OnValueChanged(const FOnAttributeChangeData& Data)
{
	SetValue(Data.NewValue);
}

void UStatsGauge::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormattingOptions.MaximumFractionalDigits =0.f;
	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return;
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);

	if (OwnerASC)
	{
		bool bFound;
		float AttributeVal = OwnerASC->GetGameplayAttributeValue(Attribute,bFound);
		SetValue(AttributeVal);

		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this,&UStatsGauge::OnValueChanged);
	}
}

