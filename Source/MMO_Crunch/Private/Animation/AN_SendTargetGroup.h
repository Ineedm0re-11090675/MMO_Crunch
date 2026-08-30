#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "AN_SendTargetGroup.generated.h"

UCLASS()
class UAN_SendTargetGroup : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditAnywhere,Category = "Gameplay Ability")
	FGameplayTagContainer TriggerGameplayCueTag;
	
	UPROPERTY(EditAnywhere,Category = "Gameplay Ability")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere,Category = "Gameplay Ability")
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere,Category = "Gameplay Ability")
	float SphereSweepRadius = 60.f;
	
	UPROPERTY(EditAnywhere,Category = "Gameplay Ability")
	TArray<FName> TargetSocketNames;

	UPROPERTY(EditAnywhere,Category = "Gameplay Ability")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere,Category = "Gameplay Ability")
	TEnumAsByte<ETeamAttitude::Type> TargetTeam = ETeamAttitude::Hostile;

	void SendLocalGameplayCue(const FHitResult& HitResult) const;
};
