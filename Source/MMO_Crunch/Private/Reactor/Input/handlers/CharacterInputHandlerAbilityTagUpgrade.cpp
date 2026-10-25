// 新增技能按键的升级/正常激活分流逻辑。
#include "CharacterInputHandlerAbilityTagUpgrade.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Character/CCharacter.h"
#include "Components/MapleInputManagerComponent.h"

void UCharacterInputHandlerAbilityTagUpgrade::HandleTriggeredEvent_Implementation(
	UMapleInputManagerComponent* Manager,
	const FInputActionValue& Value,
	const UInputAction* InputAction,
	float ElapsedTime) const
{
	if (!IsValid(Manager))
	{
		return;
	}

	const UAbilitySystemComponent* ASC = Manager->GetAbilitySystemComponent();
	const bool bUpgradeMode =
		IsValid(ASC) &&
		UpgradeModeTag.IsValid() &&
		ASC->HasMatchingGameplayTag(UpgradeModeTag);

	if (bUpgradeMode)
	{
		ACCharacter* Character = Cast<ACCharacter>(Manager->GetPawn());
		UGameplayAbility* Ability = GetAbilityInstance(Manager);

		if (IsValid(Character) && IsValid(Ability))
		{
			Character->RequestAbilityUpgrade(
				TSubclassOf<UGameplayAbility>(Ability->GetClass()));
		}

		// 升级模式下不调用父类，避免同一次按键同时激活技能。
		return;
	}

	Super::HandleTriggeredEvent_Implementation(
		Manager, Value, InputAction, ElapsedTime);
}