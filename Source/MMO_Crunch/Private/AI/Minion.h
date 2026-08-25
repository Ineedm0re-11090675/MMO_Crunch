#pragma once

#include "CoreMinimal.h"
#include "Character/CCharacter.h"
#include "Minion.generated.h"


UCLASS()
class AMinion : public ACCharacter
{
	GENERATED_BODY()
public:
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;

	bool IsActive() const;
	void Activate();

	void SetGoal(AActor* Goal);
private:
	void PickUpSkinBasedOnTeamId();

	virtual void OnRep_TeamID() override;
	
	UPROPERTY(EditDefaultsOnly,Category ="Skin")
	TMap<FGenericTeamId,USkeletalMesh*> SKinMap;

	UPROPERTY(EditDefaultsOnly,Category ="AI")
	FName GoalBlackBoardKeyName = "Goal";
};
