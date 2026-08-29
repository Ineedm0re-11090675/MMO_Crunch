#include "GAP_Dead.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAbilitySystemStatics.h"
#include "CHeroAttributeSet.h"
#include "Chaos/PBDSuspensionConstraintData.h"
#include "Engine/OverlapResult.h"

UGAP_Dead::UGAP_Dead()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	FAbilityTriggerData NewData;
	NewData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	NewData.TriggerTag = UCAbilitySystemStatics::GetDeathStatsAbilityTag();

	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatsAbilityTag());
	
	AbilityTriggers.Add(NewData);
}

void UGAP_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (K2_HasAuthority())
	{
		AActor* Killer =TriggerEventData->ContextHandle.GetEffectCauser();
		if (!Killer || !UCAbilitySystemStatics::IsHero(Killer))
		{
			Killer = nullptr;
		}
		TArray<AActor*> RewardActors = GetRewardActors();
		if (RewardActors.Num() == 0 && !Killer)
		{
			K2_EndAbility();
			return;
		}
		if (Killer && !RewardActors.Contains(Killer))
		{
			RewardActors.Add(Killer);
		}
		
		bool bFound =false;
		float SelfExp = GetAbilitySystemComponentFromActorInfo_Ensured()->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(),bFound);

		float TotalExperienceReward = BaseExperienceReward + SelfExp * ExperienceRewardPerExp;
		float TotalGoldReward = BaseGoldReward + GoldRewardPerExp * GoldRewardPerExp;

		if (Killer)
		{
			float KillerTotalExperienceReward = TotalExperienceReward * KillerRewardPortion;
			float KillerTotalGoldReward = TotalGoldReward * KillerRewardPortion;

			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(RewardEffect);
			EffectSpecHandle.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExpAttributeTag(),KillerTotalExperienceReward);
			EffectSpecHandle.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(),KillerTotalGoldReward);

			K2_ApplyGameplayEffectSpecToTarget(EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));
			TotalExperienceReward-=KillerTotalExperienceReward;
			TotalGoldReward-=KillerTotalGoldReward;
		}
		float ExpPerTarget = TotalExperienceReward /RewardActors.Num();
		float GoldPerTarget = TotalGoldReward/RewardActors.Num();
		
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(RewardEffect);
		EffectSpecHandle.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExpAttributeTag(),ExpPerTarget);
		EffectSpecHandle.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(),GoldPerTarget);

		K2_ApplyGameplayEffectSpecToTarget(EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardActors,true));
		K2_EndAbility();
	}
}

TArray<AActor*> UGAP_Dead::GetRewardActors() const
{
	TSet<AActor*> RewardActors;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return RewardActors.Array();
	}

	FCollisionObjectQueryParams CollisionParams;
	CollisionParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(RewardRange);
 	
	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(),FQuat::Identity,CollisionParams,CollisionShape))
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			const IGenericTeamAgentInterface* ResultTeamAgentInterface = Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());
			if (!ResultTeamAgentInterface || ResultTeamAgentInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
			{
				continue;
			}
			if (!UCAbilitySystemStatics::IsHero(OverlapResult.GetActor()))
			{
				continue;
			}
			RewardActors.Add(OverlapResult.GetActor());
		}
	}
	return RewardActors.Array();
}
