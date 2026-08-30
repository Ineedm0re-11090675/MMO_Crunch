#include "UpperCut.h"
#include "GameplayTagsManager.h"
#include "GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CAbilitySystemStatics.h"

UUpperCut::UUpperCut()
{
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UUpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontage =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, UpperCutMontage);
		PlayUpperCutMontage->OnBlendOut.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontage->OnCancelled.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontage->OnCompleted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontage->OnInterrupted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontage->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTag = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, GetUpperCutTag());
		WaitLaunchEventTag->EventReceived.AddDynamic(this, &UUpperCut::StartLaunching);
		WaitLaunchEventTag->ReadyForActivation();
	}
	NextComboName = NAME_None;
}

FGameplayTag UUpperCut::GetUpperCutTag()
{
	return FGameplayTag::RequestGameplayTag("ability.uppercut.launch");
}

void UUpperCut::StartLaunching(FGameplayEventData EventData)
{
	//这里牺牲了一点性能换了我觉得可以的逻辑，待修改
	bool bIsMissing = true;
	int HitTargetCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);
	if (HitTargetCount > 0)
	{
		bIsMissing = false;
	}
	if (K2_HasAuthority())
	{
		//先自推
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UppercutLaunchSpeed);
		for (int i = 0; i < HitTargetCount; i++)
		{
			FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);
			PushTarget(HitResult.GetActor(), FVector::UpVector * UppercutLaunchSpeed);
			ApplyGameplayEffectToHitResultActor(HitResult, LaunchDamageEffect,
			                                    GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}

	if (!bIsMissing)
	{
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, UGA_Combo::GetComboEventTChangedTag(), nullptr, false, false);
		WaitComboChangeEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboChangeEvent);
		WaitComboChangeEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, UCAbilitySystemStatics::GetBasicAttackPressedTag());
		WaitComboCommitEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboCommitEvent);
		WaitComboCommitEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboTargetEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, UGA_Combo::GetComboTargetGroupTag());
		WaitComboTargetEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboDamageEvent);
		WaitComboTargetEvent->ReadyForActivation();
	}
}

void UUpperCut::HandleComboChangeEvent(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;
	if (EventTag == UGA_Combo::GetComboEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}
	TArray<FName> TagName;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagName);
	NextComboName = TagName.Last();
}

void UUpperCut::HandleComboCommitEvent(FGameplayEventData Data)
{
	if (NextComboName == NAME_None)
	{
		return;
	}
	UAnimInstance* OwningAnimInstance = GetOwnerAnimInstance();
	if (OwningAnimInstance == nullptr) return;
	OwningAnimInstance->Montage_SetNextSection(OwningAnimInstance->Montage_GetCurrentSection(UpperCutMontage),
	                                           NextComboName,
	                                           UpperCutMontage
	);
}

void UUpperCut::HandleComboDamageEvent(FGameplayEventData Data)
{
	if (K2_HasAuthority())

		//先自推
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UppercutComboHoldSpeed);
	const FGenericDamageEffectDef* DamageDef = GetDamageDefFromCurrentCombo();
	if (!DamageDef) return;
	int HitTargetCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Data.TargetData);
	for (int i = 0; i < HitTargetCount; i++)
	{
		FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Data.TargetData, i);
		FVector ForceVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(
			DamageDef->PushVelocity);
		PushTarget(HitResult.GetActor(), ForceVelocity);
		ApplyGameplayEffectToHitResultActor(HitResult, DamageDef->DamageEffect,
		                                    GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}
}

const FGenericDamageEffectDef* UUpperCut::GetDamageDefFromCurrentCombo() const
{
	UAnimInstance* OwningAnimInstance = GetOwnerAnimInstance();
	if (OwningAnimInstance)
	{
		FName CurrentCombo = OwningAnimInstance->Montage_GetCurrentSection(UpperCutMontage);
		const FGenericDamageEffectDef* EffectDef = DamageMap.Find(CurrentCombo);
		return EffectDef;
	}
	return nullptr;
}
