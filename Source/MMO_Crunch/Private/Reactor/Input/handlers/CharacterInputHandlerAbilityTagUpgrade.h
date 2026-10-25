#pragma once

// 新增复用 Maple Ability Tag Handler 的角色技能输入 Handler。
#include "CoreMinimal.h"
#include "Input/Handlers/InputHandler_AbilityTag.h"
#include "CharacterInputHandlerAbilityTagUpgrade.generated.h"

// 检查升级模式；未处于升级模式时交回父类正常激活技能。
UCLASS(BlueprintType, DisplayName = "Character: Activate or Upgrade Ability by Tag")
class MMO_CRUNCH_API UCharacterInputHandlerAbilityTagUpgrade : public UInputHandler_AbilityTag
{
	GENERATED_BODY()

protected:
	// 此 Tag 应与升级模式 Handler 配置一致。
	UPROPERTY(EditDefaultsOnly, Category = "Character|Input")
	FGameplayTag UpgradeModeTag;

	virtual void HandleTriggeredEvent_Implementation(
		UMapleInputManagerComponent* Manager,
		const FInputActionValue& Value,
		const UInputAction* InputAction,
		float ElapsedTime) const override;
};