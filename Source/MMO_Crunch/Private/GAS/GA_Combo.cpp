#include "GA_Combo.h"

#include "CAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagsManager.h"

UGA_Combo::UGA_Combo()
{
	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!K2_CommitAbility()) //This Function 有点类似判断能否开火：是否正在施法，是否有足够蓝量？这里是看看能不能提交上去
	{
		K2_EndAbility();
		return;
	}
	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		if (!ComboMontage)
		{
			// Codex: Defensive check - an ability without a montage cannot
			// perform a combo, so end cleanly instead of creating a bad task.
			K2_EndAbility();
			return;
		}

		NextComboName = NAME_None;

		//允许客户段预测 + 服务器 这里的Key其实就是FPS的sequence，序列号，可以在请求出做标记，把这个key标记给这个动作
		//Create Task 这里涉及到异步流程节点
		UAbilityTask_PlayMontageAndWait* PlayMontageAndWait = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ComboMontage);
		//给Task做一些配置 ： On回馈函数 ->但这里都指向End
		PlayMontageAndWait->OnBlendOut.AddDynamic(this,&UGA_Combo::K2_EndAbility);
		PlayMontageAndWait->OnInterrupted.AddDynamic(this,&UGA_Combo::K2_EndAbility);
		PlayMontageAndWait->OnCancelled.AddDynamic(this,&UGA_Combo::K2_EndAbility);
		PlayMontageAndWait->OnCompleted.AddDynamic(this,&UGA_Combo::K2_EndAbility);
		//Carry On Task
		PlayMontageAndWait->ReadyForActivation();

		// Codex: Lesson 37 - Animation notifies send child tags such as
		// ability.combo.change.combo02; this task captures the requested next section.
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false);
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangedEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}

	SetupWaitComboInputPress();
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("ability.combo.change"));
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("ability.combo.change.end"));
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Data)
{
	if (Data.EventTag == GetComboChangedEventEndTag())
	{
		// Codex: Lesson 37 - The end notify closes the input window for the
		// current attack and prevents a stale section from being reused.
		NextComboName = NAME_None;
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(Data.EventTag, TagNames);
	if (TagNames.Num() == 0)
	{
		return;
	}

	// Codex: Lesson 37 - The final tag segment maps directly to the montage
	// section name (for example combo02).
	NextComboName = TagNames.Last();
}

void UGA_Combo::SetupWaitComboInputPress()
{
	// Codex: Lesson 38 - Re-arm the task after every press so input can be
	// buffered repeatedly during the montage.
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &UGA_Combo::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupWaitComboInputPress();
	TryCommitCombo();
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)
	{
		return;
	}

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (!OwnerAnimInst || !ComboMontage)
	{
		return;
	}

	const FName CurrentSection = OwnerAnimInst->Montage_GetCurrentSection(ComboMontage);
	if (CurrentSection != NAME_None)
	{
		// Codex: Lesson 38 - Montage_SetNextSection changes the transition
		// target without restarting the currently playing montage.
		OwnerAnimInst->Montage_SetNextSection(CurrentSection, NextComboName, ComboMontage);
	}
}
