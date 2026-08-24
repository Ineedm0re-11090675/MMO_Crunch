#include "GA_Combo.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "GameplayTagsManager.h"
UGA_Combo::UGA_Combo()
{
	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData* TriggerEventData)
{
	//Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!K2_CommitAbility()) //This Function 有点类似判断能否开火：是否正在施法，是否有足够蓝量？这里是看看能不能提交上去
	{
		K2_EndAbility();
		return;
	}
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		//允许客户段预测 + 服务器 这里的Key其实就是FPS的sequence，序列号，可以在请求出做标记，把这个key标记给这个动作
		//Create Task 这里涉及到异步流程节点
		UAbilityTask_PlayMontageAndWait* PlayMontageAndWait =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);
		//给Task做一些配置 ： On回馈函数 ->但这里都指向End
		PlayMontageAndWait->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayMontageAndWait->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayMontageAndWait->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayMontageAndWait->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		//Carry On Task
		PlayMontageAndWait->ReadyForActivation();


		UAbilityTask_WaitGameplayEvent* WaitGameplayEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, // 当前 Ability
			GetComboEventTChangedTag(), // 要监听的 Event Tag
			nullptr, // 监听目标，nullptr = 自身
			false, // false = 可重复触发
			false // false = 允许非精确/层级 Tag 匹配
		);
		WaitGameplayEvent->EventReceived.AddDynamic(this,&UGA_Combo::ComboEventChangedReceived);
		WaitGameplayEvent->ReadyForActivation();
		
	}
	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetingEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			GetComboTargetGroupTag()
			);
		WaitTargetingEvent->EventReceived.AddDynamic(this,&UGA_Combo::HandleTargetGroupReceive);
		WaitTargetingEvent->ReadyForActivation();
	}
	SetupWaitInputPress();
}

FGameplayTag UGA_Combo::GetComboEventTChangedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change");
}
 
FGameplayTag UGA_Combo::GetComboEventEndTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change.end");
}

FGameplayTag UGA_Combo::GetComboTargetGroupTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.damage");
}

void UGA_Combo::SetupWaitInputPress()
{
	//这里的第二个入参是 检测当前这个按键是否持续按下？创建这个等待任务时，要不要先检查“这个技能对应的输入现在是不是已经处于按下状态”。
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this,&UGA_Combo::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}
void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupWaitInputPress();
	TryCommitCombo();
}

void UGA_Combo::HandleTargetGroupReceive(FGameplayEventData Data)
{
	TArray<FHitResult> HitResults = GetHitResultsFromSweepLocationTargetData(Data.TargetData,TargetSweepSphereRadius);  
	for (const FHitResult& HitResult : HitResults)
	{
		TSubclassOf<UGameplayEffect> GameplayEffect = GetCurrentComboDamageEffect();
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect,GetAbilityLevel(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo()));

		//将HitResult加入到GE的context信息内部，让BluePrint能读到HitResult，然后让GC读到FVX方向
		FGameplayEffectContextHandle ContextHandle =MakeEffectContext(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo());
		ContextHandle.AddHitResult(HitResult);
		EffectSpecHandle.Data->SetContext(ContextHandle);
		
		ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(),CurrentActorInfo,
			CurrentActivationInfo,
			EffectSpecHandle,
			UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor())
			);
		
	}
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)
	{
		return;
	}
	UAnimInstance* OwningAnimInstance = GetOwnerAnimInstance();
	if (OwningAnimInstance == nullptr) return;
	OwningAnimInstance->Montage_SetNextSection(OwningAnimInstance->Montage_GetCurrentSection(ComboMontage),
		NextComboName,
		ComboMontage
		);
	
}

TSubclassOf<UGameplayEffect> UGA_Combo::GetCurrentComboDamageEffect() const
{
	UAnimInstance* OwningAnimInstance = GetOwnerAnimInstance();
	if (OwningAnimInstance)
	{
		FName CurrentSectionName = OwningAnimInstance->Montage_GetCurrentSection(ComboMontage);
		const TSubclassOf<UGameplayEffect>* FoundGameplayEffect = DamageEffectsMap.Find(CurrentSectionName);
		if (FoundGameplayEffect)
		{
			return *FoundGameplayEffect;
		}
	}
	return DefaultDamageEffect;
}

void UGA_Combo::ComboEventChangedReceived(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;
	if (EventTag == GetComboEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}
	TArray<FName> TagName;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag,TagName);
	NextComboName = TagName.Last();
}

