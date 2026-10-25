#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "CharacterProgressionComponent.generated.h"

class UAbilitySystemComponent;
class UCurveTable;
class UGameplayAbility;

// 从旧ASC中分得逻辑，管理 Character 的经验、等级、升级点和技能升级规则，不替代 Maple ASC。
UCLASS(ClassGroup=(Crunch), meta=(BlueprintSpawnableComponent))
class MMO_CRUNCH_API UCharacterProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterProgressionComponent();

	// 由 Character 在 Maple ASC 初始化后传入实际使用的 ASC。
	bool InitializeFromASC(UAbilitySystemComponent* InASC);

	// 查询角色等级是否已达到经验曲线规定的上限。
	bool IsMaxLevel() const;

	// 仅允许服务器调用；技能类必须在白名单中。
	bool TryUpgradeAbility(TSubclassOf<UGameplayAbility> AbilityClass);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 把 GAS Experience 属性变化转换为角色等级和升级点。
	// ExperienceUpdate函数
	void HandleExperienceChanged(const FOnAttributeChangeData& ChangeData);

	// 按旧经验曲线规则更新 Level、升级点和经验区间。
	// 从  ExperienceUpdate函数 拆出来的辅助函数
	void RefreshProgressionFromExperience(float CurrentExperience);

	// 组件结束时解除 ASC 委托，避免留下悬空绑定。
	// 安全性考虑
	void UnbindExperienceDelegate();

	// 从旧 PA 配置迁入的经验曲线引用。
	UPROPERTY(EditDefaultsOnly, Category="Crunch|Progression")
	TObjectPtr<UCurveTable> ExperienceCurveTable;

	UPROPERTY(EditDefaultsOnly, Category="Crunch|Progression")
	FName ExperienceRowName = TEXT("ExperienceNeededToReachLevel");

	// 只允许这些技能消耗升级点，避免客户端请求升级被动或非成长技能。
	UPROPERTY(EditDefaultsOnly, Category="Crunch|Progression")
	TArray<TSubclassOf<UGameplayAbility>> UpgradeableAbilityClasses;

	UPROPERTY(EditDefaultsOnly, Category="Crunch|Progression")
	int32 MaxAbilityLevel = 4;

	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	FDelegateHandle ExperienceChangedHandle;
};