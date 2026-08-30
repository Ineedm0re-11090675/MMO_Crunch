#include "CAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CAbilitySystemStatics.h"
#include "CHeroAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "PA_AbilitySystemGenerics.h"
#include "GAS/CAttributeSet.h"
#include "Conditions/MovieSceneScalabilityCondition.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this, &UCAbilitySystemComponent::UpdateHealth);
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UCAbilitySystemComponent::UpdateMana);
	GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this, &UCAbilitySystemComponent::UpdateExperience);
	GenericCancelInputID = (int32)ECAbilityInputId::Cancel;
	GenericConfirmInputID = (int32)ECAbilityInputId::Confirm;
	
}

void UCAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttributes();
	ApplyInitialEffects();
	GiveInitialAbility();
}

void UCAbilitySystemComponent::InitializeBaseAttributes()
{
	if (!AbilitySystemGenerics || !AbilitySystemGenerics->GetBaseStatsDataTable()) return;

	const UDataTable* BaseStatsDataTable =AbilitySystemGenerics->GetBaseStatsDataTable(); 
	const FHeroBaseStats* BaseStats = nullptr;

	for (const TPair<FName,uint8*>& DataPair : BaseStatsDataTable->GetRowMap())
	{
		BaseStats = BaseStatsDataTable->FindRow<FHeroBaseStats>(DataPair.Key,"");
		if (BaseStats && BaseStats->Class == GetOwner()->GetClass())
		{
			break; 
		}
	}
	if (BaseStats)
	{
		SetNumericAttributeBase(UCAttributeSet::GetMaxHealthAttribute(),BaseStats->BaseMaxHealth);
		SetNumericAttributeBase(UCAttributeSet::GetMaxManaAttribute(),BaseStats->BaseMaxMana);
		SetNumericAttributeBase(UCAttributeSet::GetArmorAttribute(),BaseStats->BaseArmor);
		SetNumericAttributeBase(UCAttributeSet::GetAttackDamageAttribute(),BaseStats->BaseAttackDamage);
		SetNumericAttributeBase(UCAttributeSet::GetMoveSpeedAttribute(),BaseStats->BaseMoveSpeed);
		
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthAttribute(),BaseStats->Strength);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceAttribute(),BaseStats->Intelligence);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceGrowRateAttribute(),BaseStats->IntelligenceGrowRate);
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthGrowRateAttribute(),BaseStats->StrengthGrowRate);
		
	}

	const FRealCurve* ExpCurve = AbilitySystemGenerics->GetExpCurve();
	if (ExpCurve)
	{
		int MaxLevel = ExpCurve->GetNumKeys();
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelAttribute(),MaxLevel);

		float MaxLevelExp =ExpCurve->GetKeyValue(ExpCurve->GetLastKeyHandle());
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelExperienceAttribute(),MaxLevelExp);
		
	}
	UpdateExperience(FOnAttributeChangeData());
}


void UCAbilitySystemComponent::ApplyInitialEffects()
{
	/*Explain ：为什么写在SystemComponent里也能影响到Attribute？
	Character / PlayerState
	│
	├── AbilitySystemComponent
	│
	└── AttributeSet
	*/

	if (!AbilitySystemGenerics) return;
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	for (const TSubclassOf<UGameplayEffect>& EffectClass : AbilitySystemGenerics->GetInitialEffect())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass , 1 , MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::GiveInitialAbility()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	for (const TPair<ECAbilityInputId,TSubclassOf<UGameplayAbility>>& AbilityPair : GameplayAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,0,(int32)AbilityPair.Key,nullptr));
	}
	
	for (const TPair<ECAbilityInputId,TSubclassOf<UGameplayAbility>>& AbilityPair : BasicGameplayAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,1,(int32)AbilityPair.Key,nullptr));
	}
	if (!AbilitySystemGenerics) return;
	for (const TSubclassOf<UGameplayAbility> PassiveAbilityClass: AbilitySystemGenerics->GetPassiveAbility() )
	{
		GiveAbility(FGameplayAbilitySpec(PassiveAbilityClass, 1,-1,nullptr));
	}
}

void UCAbilitySystemComponent::ApplyFullStatsEffect()
{
	if (!AbilitySystemGenerics) return;
	AuthApplyGameplayEffect(AbilitySystemGenerics->GetFullStatsEffect());
}

bool UCAbilitySystemComponent::bIsMaxLevel() const
{
	bool bFound;
	float CurrentLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(),bFound);
	float MaxLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetMaxLevelAttribute(),bFound);
	return CurrentLevel >= MaxLevel;
	
}

void UCAbilitySystemComponent::Server_UpgradeAbilityWithID_Implementation(ECAbilityInputId InputID)
{
	bool bFound;
	float UpgradePoint = GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(),bFound);
	if (!bFound || UpgradePoint <=0) return;
	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID((int32)InputID);
	if (!AbilitySpec || UCAbilitySystemStatics::IsAbilityAtMaxLevel(*AbilitySpec) ) return;

	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(),UpgradePoint-1);

	AbilitySpec->Level+=1;
	MarkAbilitySpecDirty(*AbilitySpec);
	Client_AbilitySpecLevelUpdated(AbilitySpec->Handle, AbilitySpec->Level);
}

bool UCAbilitySystemComponent::Server_UpgradeAbilityWithID_Validate(ECAbilityInputId InputID)
{
	return true; 
}


void UCAbilitySystemComponent::Client_AbilitySpecLevelUpdated_Implementation(FGameplayAbilitySpecHandle Handle,int NewLevel)
{
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
	if (!Spec) return;
	Spec->Level = NewLevel;
	AbilitySpecDirtiedCallbacks.Broadcast(*Spec);
}

const TMap<ECAbilityInputId, TSubclassOf<UGameplayAbility>>& UCAbilitySystemComponent::GetAbility() const
{
	return GameplayAbilities;
}

void UCAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect ,Level , MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::UpdateHealth(const FOnAttributeChangeData& ChangedData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound =false;
	float MaxHealth = GetGameplayAttributeValue(UCAttributeSet::GetMaxHealthAttribute(),bFound);
	if (bFound && ChangedData.NewValue >= MaxHealth)
	{
		 if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetFullHealthStatsAbilityTag()))
		 {
			 AddLooseGameplayTag(UCAbilitySystemStatics::GetFullHealthStatsAbilityTag());
		 }
	}else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetFullHealthStatsAbilityTag());
	}
	if (ChangedData.NewValue <= 0 )
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetEmptyHealthStatsAbilityTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetEmptyHealthStatsAbilityTag());
			
			if (AbilitySystemGenerics && AbilitySystemGenerics->GetDeathEffect())
			{
				AuthApplyGameplayEffect(AbilitySystemGenerics->GetDeathEffect());
			}
			FGameplayEventData DeadAbilityEventData;
			//可以从Health变化的Data里读取有效信息
			if (ChangedData.GEModData)
				DeadAbilityEventData.ContextHandle = ChangedData.GEModData->EffectSpec.GetContext();
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(),UCAbilitySystemStatics::GetDeathStatsAbilityTag(),DeadAbilityEventData);
 		}
	}else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetEmptyHealthStatsAbilityTag());
	}
}

void UCAbilitySystemComponent::UpdateMana(const FOnAttributeChangeData& ChangedData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound =false;
	float MaxMana = GetGameplayAttributeValue(UCAttributeSet::GetMaxManaAttribute(),bFound);
	if (bFound && ChangedData.NewValue >= MaxMana)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetFullManaStatsAbilityTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetFullManaStatsAbilityTag());
		}
	}else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetFullManaStatsAbilityTag());
	}
	if (ChangedData.NewValue <= 0 )
	{
	
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetEmptyManaStatsAbilityTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetEmptyManaStatsAbilityTag());
		}
	}else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetEmptyManaStatsAbilityTag());
	}
}

void UCAbilitySystemComponent::UpdateExperience(const FOnAttributeChangeData& ChangedData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (bIsMaxLevel()) return;
	if (!AbilitySystemGenerics) return;

	float CurrentExp = ChangedData.NewValue;

	const FRealCurve* ExpCurve = AbilitySystemGenerics->GetExpCurve();
	if (!ExpCurve)
	{
		UE_LOG(LogTemp,Warning,TEXT("Error : cant find Curve"));
		return;
	}


	float PreLevelExp = 0;
	float NextLevelExp = 0;
	int NewLevel =1;
	for(auto Iter =ExpCurve->GetKeyHandleIterator() ; Iter ; ++Iter)
	{
		float ExpNeeded =ExpCurve->GetKeyValue(*Iter);
		if (ExpNeeded > CurrentExp)
		{
			NextLevelExp = ExpNeeded;
			break;
		}
		PreLevelExp = ExpNeeded;
		NewLevel = Iter.GetIndex()+1;
	}
 
	float CurrentLevel = GetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute());
	float CurrentUpgradePoint = GetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute());

	float LevelUpgrade = NewLevel -CurrentLevel;
	//这个是升级点的意思，加点数量 
	float NewUpgradePoint = CurrentUpgradePoint + LevelUpgrade;

	SetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute(),NewLevel);
	SetNumericAttributeBase(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(),PreLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetNextLevelExperienceAttribute(),NextLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(),NewUpgradePoint);
	
}
