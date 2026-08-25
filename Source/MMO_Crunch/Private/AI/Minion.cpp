#include "Minion.h"
#include "CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void AMinion::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	Super::SetGenericTeamId(TeamID);
	PickUpSkinBasedOnTeamId();
}

bool AMinion::IsActive() const
{
	return !IsDead();
}

void AMinion::Activate()
{
	SpawnImmediately();
}

void AMinion::SetGoal(AActor* Goal)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsObject(GoalBlackBoardKeyName, Goal);
		}
	}
}

void AMinion::PickUpSkinBasedOnTeamId()
{
	USkeletalMesh** Skin = SKinMap.Find(GetGenericTeamId());
	if (Skin)
	{
		GetMesh()->SetSkeletalMesh(*Skin);
	}
}

void AMinion::OnRep_TeamID()
{
	PickUpSkinBasedOnTeamId();
}
