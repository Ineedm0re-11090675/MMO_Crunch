#include "AbilityGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CAbilitySystemStatics.h" 
#include "GAS/CAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"


void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	//这个函数在AbilityListView调用AddItem时触发， 
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	AbilityCDO = Cast<UGameplayAbility>(ListItemObject);

	float CooldownDuration =UCAbilitySystemStatics::GetStaticCooldownDurationFromAbility(AbilityCDO);
	float Cost =UCAbilitySystemStatics::GetStaticCostFromAbility(AbilityCDO);

	CostText->SetText(FText::AsNumber(Cost));
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParaName,0);
}

void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct(); 
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);

	UAbilitySystemComponent* OwnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (OwnASC)
	{
		OwnASC->AbilityCommittedCallbacks.AddUObject(this,&UAbilityGauge::AbilityCommittedCallback); 
		//之前在ASC做了Update之后的DirtyCall，就可以在这里相应了
		OwnASC->AbilitySpecDirtiedCallbacks.AddUObject(this,&UAbilityGauge::AbilitySpecUpdated);
		OwnASC->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetUpgradePointAttribute()).AddUObject(this,&UAbilityGauge::UpgradePointUpdated);
		OwnASC->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this,&UAbilityGauge::ManaUpdated);
		
		//初始1级在此手动更新
		bool bFound = false;
		float UpgradePoint = OwnASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(),bFound);
		if (bFound)
		{
			FOnAttributeChangeData ChangeData;
			ChangeData.NewValue = UpgradePoint;
			UpgradePointUpdated(ChangeData);
		}
		 
	}

	OwningASC =OwnASC;

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

const FGameplayAbilitySpec* UAbilityGauge::GetAbilitySpec()
{
	if (!CachedAbilitySpec)
	{
		if (AbilityCDO && OwningASC)
		{
			CachedAbilitySpec =OwningASC->FindAbilitySpecFromClass(AbilityCDO->GetClass());
		}
	}
	return CachedAbilitySpec;
}

void UAbilityGauge::AbilitySpecUpdated(const FGameplayAbilitySpec& Spec)
{
	if (Spec.Ability != AbilityCDO) return;

	bIsAbilityLearned =  Spec.Level > 0;
	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParaName,Spec.Level);
	UpdateCanCast();

	 float NewCooldownDuration = UCAbilitySystemStatics::GetCooldownDurationFor(Spec.Ability,*OwningASC,Spec.Level);
	 float NewCost =UCAbilitySystemStatics::GetManaCostFor(Spec.Ability,*OwningASC,Spec.Level);

	CooldownDurationText->SetText(FText::AsNumber(NewCooldownDuration) );
	CostText->SetText(FText::AsNumber(NewCost));
}

void UAbilityGauge::UpdateCanCast()
{
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	bool bCanCast = bIsAbilityLearned;
	if (AbilitySpec)
	{
		if (OwningASC && !UCAbilitySystemStatics::CheckAbilityCost(*AbilitySpec, *OwningASC))
		{
			bCanCast = false;
		}
	}
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CanCastParaName,bCanCast?1.f:0.f);
}

void UAbilityGauge::UpgradePointUpdated(const FOnAttributeChangeData& Data)
{
	bool HasUpgradePoint = Data.NewValue >0;
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	if (AbilitySpec)
	{
		if (UCAbilitySystemStatics::IsAbilityAtMaxLevel(*AbilitySpec))
		{
			Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradePointAvailableParaName,0.f);
			return;
		}
	}
	Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradePointAvailableParaName,HasUpgradePoint?1.f:0.f);
}

void UAbilityGauge::ManaUpdated(const FOnAttributeChangeData& Data)
{
	//消耗蓝 然后更新该技能是非应该为灰色
	UpdateCanCast();
}
