#include "AbilityToolTip.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UAbilityToolTip::SetAbilityInfo(const FName& AbilityName, UTexture2D* AbilityTexture,
                                     const FText& AbilityDescription, float AbilityCooldown, float AbilityCost)
{
	AbilityNameText->SetText(FText::FromName(AbilityName));
	AbilityIcon->SetBrushFromTexture(AbilityTexture);
	AbilityDescriptionText->SetText(AbilityDescription);
	
	
	FNumberFormattingOptions NumberFormattingOptions;
	NumberFormattingOptions.MaximumFractionalDigits = 0;

	AbilityCooldownText->SetText(FText::AsNumber(AbilityCooldown, &NumberFormattingOptions));
	AbilityCostText->SetText(FText::AsNumber(AbilityCost, &NumberFormattingOptions));
	
}
