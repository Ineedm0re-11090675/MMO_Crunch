// 新增角色升级模式输入 Handler 的实现。
#include "CharacterInputHandlerUpgradeModeTag.h"

#include "AbilitySystemComponent.h"
#include "Components/MapleInputManagerComponent.h"

UCharacterInputHandlerUpgradeModeTag::UCharacterInputHandlerUpgradeModeTag()
{
	TriggerEvents = {
		ETriggerEvent::Started,
		ETriggerEvent::Completed,
		ETriggerEvent::Canceled
	};
}

void UCharacterInputHandlerUpgradeModeTag::SetUpgradeMode(
	UMapleInputManagerComponent* Manager, bool bEnabled) const
{
	if (!IsValid(Manager) || !UpgradeModeTag.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = Manager->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	if (bEnabled)
	{
		ASC->AddLooseGameplayTag(UpgradeModeTag);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(UpgradeModeTag);
	}
}

void UCharacterInputHandlerUpgradeModeTag::HandleStartedEvent_Implementation(
	UMapleInputManagerComponent* Manager,
	const FInputActionValue&,
	const UInputAction*) const
{
	SetUpgradeMode(Manager, true);
}

void UCharacterInputHandlerUpgradeModeTag::HandleCompletedEvent_Implementation(
	UMapleInputManagerComponent* Manager,
	const FInputActionValue&,
	const UInputAction*,
	float) const
{
	SetUpgradeMode(Manager, false);
}

void UCharacterInputHandlerUpgradeModeTag::HandleCancelledEvent_Implementation(
	UMapleInputManagerComponent* Manager,
	const FInputActionValue&,
	const UInputAction*,
	float) const
{
	SetUpgradeMode(Manager, false);
}