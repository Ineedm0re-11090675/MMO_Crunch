#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "CGameplayAbility.generated.h"

UCLASS()
class UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
protected:
	UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultsFromSweepLocationTargetData(FGameplayAbilityTargetDataHandle& TargetDataHandle,float SphereRadius =20.f,ETeamAttitude::Type TargetTeam =ETeamAttitude::Hostile,bool bShowDebugSphere = false,bool bIgnoreSelf = true) const;
};
