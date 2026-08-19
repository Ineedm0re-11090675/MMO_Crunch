#include "GA_Combo.h"

#include "CAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

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
	}
}
