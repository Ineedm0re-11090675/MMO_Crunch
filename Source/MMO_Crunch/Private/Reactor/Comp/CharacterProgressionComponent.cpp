#include "CharacterProgressionComponent.h"

#include "AbilitySystemComponent.h"
#include "Engine/CurveTable.h"
#include "GameplayAbilitySpec.h"
#include "Reactor/GAS/CCharacterAttributeSet.h"

// 成长计算由属性变化驱动，不需要每帧 Tick。
UCharacterProgressionComponent::UCharacterProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Character 应在 MapleGAS 完成 DA 初始化后调用本函数。
bool UCharacterProgressionComponent::InitializeFromASC(UAbilitySystemComponent* InASC)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsValid(InASC))
	{
		return false;
	}

	if (!InASC->GetSet<UCCharacterAttributeSet>() || !IsValid(ExperienceCurveTable))
	{
		return false;
	}

	const FRealCurve* Curve =
		ExperienceCurveTable->FindCurve(ExperienceRowName, TEXT("Crunch progression"));

	if (!Curve || Curve->GetNumKeys() == 0)
	{
		return false;
	}

	UnbindExperienceDelegate();
	CachedASC = InASC;

	// 沿用旧系统以曲线 key 数量作为最大等级的约定。
	const float MaxLevel = static_cast<float>(Curve->GetNumKeys());
	const float MaxExperience = Curve->GetKeyValue(Curve->GetLastKeyHandle());

	InASC->SetNumericAttributeBase(UCCharacterAttributeSet::GetMaxLevelAttribute(), MaxLevel);
	InASC->SetNumericAttributeBase(
		UCCharacterAttributeSet::GetMaxLevelExperienceAttribute(), MaxExperience);

	ExperienceChangedHandle =
		InASC->GetGameplayAttributeValueChangeDelegate(
			UCCharacterAttributeSet::GetExperienceAttribute())
		.AddUObject(this, &ThisClass::HandleExperienceChanged);

	// 处理 DA 中已有的初始 Experience；正常新角色从 0 开始。
	RefreshProgressionFromExperience(
		InASC->GetNumericAttribute(UCCharacterAttributeSet::GetExperienceAttribute()));

	return true;
}

// 经验变化后更新等级、升级点以及 UI 使用的经验区间。
void UCharacterProgressionComponent::HandleExperienceChanged(
	const FOnAttributeChangeData& ChangeData)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RefreshProgressionFromExperience(ChangeData.NewValue);
	}
}

// 沿用旧逻辑：经验曲线每经过一个等级门槛，增加一个升级点。
void UCharacterProgressionComponent::RefreshProgressionFromExperience(float CurrentExperience)
{
	UAbilitySystemComponent* ASC = CachedASC.Get();
	const FRealCurve* Curve = IsValid(ExperienceCurveTable)
		? ExperienceCurveTable->FindCurve(ExperienceRowName, TEXT("Crunch progression"))
		: nullptr;

	if (!ASC || !Curve || Curve->GetNumKeys() == 0)
	{
		return;
	}

	float PreviousLevelExperience = 0.0f;
	float NextLevelExperience = 0.0f;
	int32 CurveLevel = 1;

	for (auto It = Curve->GetKeyHandleIterator(); It; ++It)
	{
		const float Threshold = Curve->GetKeyValue(*It);

		if (Threshold > CurrentExperience)
		{
			NextLevelExperience = Threshold;
			break;
		}

		PreviousLevelExperience = Threshold;
		CurveLevel = It.GetIndex() + 1;
	}

	const int32 MaxLevel = Curve->GetNumKeys();
	const int32 CurrentLevel = FMath::Max(
		1, FMath::FloorToInt(
			ASC->GetNumericAttribute(UCCharacterAttributeSet::GetLevelAttribute())));

	// 经验减少时暂不让角色降级；降级规则需要另行定案。
	const int32 NewLevel = FMath::Clamp(FMath::Max(CurrentLevel, CurveLevel), 1, MaxLevel);
	const int32 GainedLevels = NewLevel - CurrentLevel;

	const float CurrentUpgradePoints =
		ASC->GetNumericAttributeBase(UCCharacterAttributeSet::GetUpgradePointAttribute());

	ASC->SetNumericAttributeBase(UCCharacterAttributeSet::GetLevelAttribute(), NewLevel);
	ASC->SetNumericAttributeBase(
		UCCharacterAttributeSet::GetPrevLevelExperienceAttribute(), PreviousLevelExperience);
	ASC->SetNumericAttributeBase(
		UCCharacterAttributeSet::GetNextLevelExperienceAttribute(), NextLevelExperience);

	if (GainedLevels > 0)
	{
		ASC->SetNumericAttributeBase(
			UCCharacterAttributeSet::GetUpgradePointAttribute(),
			CurrentUpgradePoints + GainedLevels);
	}
}

// 角色是否达到曲线配置的等级上限。
bool UCharacterProgressionComponent::IsMaxLevel() const
{
	const UAbilitySystemComponent* ASC = CachedASC.Get();
	if (!ASC)
	{
		return false;
	}

	const float MaxLevel =
		ASC->GetNumericAttribute(UCCharacterAttributeSet::GetMaxLevelAttribute());
	const float CurrentLevel =
		ASC->GetNumericAttribute(UCCharacterAttributeSet::GetLevelAttribute());

	return MaxLevel > 0.0f && CurrentLevel >= MaxLevel;
}

// 服务器验证升级点、目标技能及技能等级，再让 ASC 修改并复制 spec。
bool UCharacterProgressionComponent::TryUpgradeAbility(
	TSubclassOf<UGameplayAbility> AbilityClass)
{
	UAbilitySystemComponent* ASC = CachedASC.Get();

	if (!GetOwner() || !GetOwner()->HasAuthority() || !ASC || !AbilityClass)
	{
		return false;
	}

	if (!UpgradeableAbilityClasses.Contains(AbilityClass))
	{
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(AbilityClass);
	if (!AbilitySpec || AbilitySpec->Level >= MaxAbilityLevel)
	{
		return false;
	}

	const float UpgradePoints =
		ASC->GetNumericAttributeBase(UCCharacterAttributeSet::GetUpgradePointAttribute());

	if (UpgradePoints < 1.0f)
	{
		return false;
	}

	ASC->SetNumericAttributeBase(
		UCCharacterAttributeSet::GetUpgradePointAttribute(), UpgradePoints - 1.0f);

	++AbilitySpec->Level;
	ASC->MarkAbilitySpecDirty(*AbilitySpec);
	return true;
}

// 解除本组件注册在 ASC 上的经验变化回调。
void UCharacterProgressionComponent::UnbindExperienceDelegate()
{
	UAbilitySystemComponent* ASC = CachedASC.Get();

	if (ASC && ExperienceChangedHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UCCharacterAttributeSet::GetExperienceAttribute())
			.Remove(ExperienceChangedHandle);
	}

	ExperienceChangedHandle.Reset();
	CachedASC.Reset();
}

void UCharacterProgressionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindExperienceDelegate();
	Super::EndPlay(EndPlayReason);
}