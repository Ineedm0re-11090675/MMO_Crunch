#pragma once

// 新增角色升级模式输入 Handler。
#include "CoreMinimal.h"
#include "MapleInputHandler.h"
#include "CharacterInputHandlerUpgradeModeTag.generated.h"

class UMapleInputManagerComponent;

// 新增在按下、松开和取消时管理升级模式 Tag 的 Handler。
UCLASS(BlueprintType, DisplayName = "Character: Upgrade Mode Tag")
class MMO_CRUNCH_API UCharacterInputHandlerUpgradeModeTag : public UMapleInputHandler
{
	GENERATED_BODY()

public:
	// 设置本 Handler 需要处理的输入事件。
	UCharacterInputHandlerUpgradeModeTag();

protected:
	// 在 IS_Player 中配置升级模式 Tag。
	UPROPERTY(EditDefaultsOnly, Category = "Character|Input")
	FGameplayTag UpgradeModeTag;

	// 按下时添加升级模式 Tag。
	virtual void HandleStartedEvent_Implementation(
		UMapleInputManagerComponent* Manager,
		const FInputActionValue& Value,
		const UInputAction* InputAction) const override;

	// 松开或取消时移除升级模式 Tag。
	virtual void HandleCompletedEvent_Implementation(
		UMapleInputManagerComponent* Manager,
		const FInputActionValue& Value,
		const UInputAction* InputAction,
		float ElapsedTime) const override;

	virtual void HandleCancelledEvent_Implementation(
		UMapleInputManagerComponent* Manager,
		const FInputActionValue& Value,
		const UInputAction* InputAction,
		float ElapsedTime) const override;

private:
	// 集中处理升级模式 Tag 的添加与移除。
	void SetUpgradeMode(UMapleInputManagerComponent* Manager, bool bEnabled) const;
};