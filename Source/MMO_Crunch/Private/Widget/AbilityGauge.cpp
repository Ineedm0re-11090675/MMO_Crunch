#include "AbilityGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAbilitySystemStatics.h"
void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	//这个函数在AbilityListView调用AddItem时触发， 
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	AbilityCDO = Cast<UGameplayAbility>(ListItemObject);

	float CooldownDuration =UCAbilitySystemStatics::GetStaticCooldownDurationFromAbility(AbilityCDO);
	float Cost =UCAbilitySystemStatics::GetStaticCostFromAbility(AbilityCDO);

	CostText->SetText(FText::AsNumber(Cost));
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration)); 
}

void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct(); 
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);

	UAbilitySystemComponent* OwnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnASC)
	{
		OwnASC->AbilityCommittedCallbacks.AddUObject(this,&UAbilityGauge::AbilityCommittedCallback); 
	}

	WholeNumericFormattingOptions.MaximumFractionalDigits =0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits =2;
	
}

void UAbilityGauge::ConfigureAbilityWithWidgetData(const FAbilityWidgetData* WidgetData)
{
	if (Icon && WidgetData)
	{
		Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParaName,WidgetData->Icon.LoadSynchronous());
	}
}

void UAbilityGauge::AbilityCommittedCallback(UGameplayAbility* Ability)
{
	//CDO是一个默认对象实例，Ability本身是一个具体的技能，所以在这里必须抽象化来判断
	if (Ability->GetClass()->GetDefaultObject() == AbilityCDO)
	{
		float CooldownTimeRemaining = 0.f;
		float CooldownDuration = 0.f;
		Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(),Ability->GetCurrentActorInfo(),CooldownTimeRemaining,CooldownDuration);

		StartCooldown(CooldownTimeRemaining,CooldownDuration);
	}
}

void UAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;
	
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));

	CooldownCounterText->SetVisibility(ESlateVisibility::Visible);
	
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle,this,&UAbilityGauge::CooldownFinish,CooldownDuration);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle,this,&UAbilityGauge::UpdateCooldown,CooldownUpdateInterval,true,0.f);
}

void UAbilityGauge::CooldownFinish()
{
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;

	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParaName,1.f);
}

void UAbilityGauge::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdateInterval;

	FNumberFormattingOptions* NumericFormattingOptions = CachedCooldownTimeRemaining >1 ? &WholeNumericFormattingOptions : & TwoDigitNumberFormattingOptions;
	
	CooldownCounterText->SetText(FText::AsNumber(CachedCooldownTimeRemaining,NumericFormattingOptions));

	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParaName,1.f-CachedCooldownTimeRemaining/CachedCooldownDuration);
}
