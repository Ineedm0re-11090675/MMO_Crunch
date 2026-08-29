#include "LevelWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CHeroAttributeSet.h"

void ULevelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormatOptions.SetMaximumFractionalDigits(0);

	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return;

	UAbilitySystemComponent* OwnerAbilitySystemComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!OwnerAbilitySystemComp) return;

	OwnASC = OwnerAbilitySystemComp;

	UpdateGauge(FOnAttributeChangeData());
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this,&ULevelWidget::UpdateGauge);
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetPrevLevelExperienceAttribute()).AddUObject(this,&ULevelWidget::UpdateGauge);
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetNextLevelExperienceAttribute()).AddUObject(this,&ULevelWidget::UpdateGauge);
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetLevelAttribute()).AddUObject(this,&ULevelWidget::UpdateGauge);
}

void ULevelWidget::UpdateGauge(const FOnAttributeChangeData& Data)
{
	bool bFound;
	float CurrentExp = OwnASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(),bFound);
	if (!bFound) return;
	float PreExp = OwnASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(),bFound);
	if (!bFound) return;
	float NextExp =OwnASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetNextLevelExperienceAttribute(),bFound);
	if (!bFound) return;
	float CurrentLevel =OwnASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(),bFound);
	if (!bFound) return;

	Level->SetText(FText::AsNumber(CurrentLevel,&NumberFormatOptions));

	float Progress = CurrentExp - PreExp;
	float LevelExpAmt =NextExp - PreExp;

	float Percent =Progress / LevelExpAmt;
	if (NextExp == 0)
	{
		Percent = 1; 
	}

	if (LevelProgress)
	{
		LevelProgress->GetDynamicMaterial()->SetScalarParameterValue(PercentMaterialParaName,Percent); 
	}
}
