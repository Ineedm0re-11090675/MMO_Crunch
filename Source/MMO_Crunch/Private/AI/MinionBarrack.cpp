#include "MinionBarrack.h"
#include "Minion.h"
#include "GameFramework/PlayerStart.h"

void AMinionBarrack::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(SpawnMinionsIntervalTimerHandle,this,&AMinionBarrack::SpawnNewGroup ,SpawnInterval,true);
	}
}

const APlayerStart* AMinionBarrack::GetNextSpawnIndex()
{
	if (SpawnSpots.Num() == 0) return nullptr;

	++NextSpawnIndex;

	if (NextSpawnIndex >= SpawnSpots.Num())
	{
		NextSpawnIndex = 0;
	}
	 return SpawnSpots[NextSpawnIndex];
}

AMinion* AMinionBarrack::GetNextValueMinion() const
{
	for (AMinion* Minion : MinionPool)
	{
		if (Minion->IsActive())
		{
			return Minion;
		}
	}
	return nullptr;
}

void AMinionBarrack::SpawnMinions(int Amt)
{
	for (int i = 0; i < Amt; i++)
	{
		FTransform SpawnTransform =GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnIndex())
		{
			SpawnTransform = NextSpawnSpot->GetTransform();
		}

		AMinion* NewMinion = GetWorld()->SpawnActorDeferred<AMinion>(MinionClass,SpawnTransform,this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn); 
		NewMinion->SetGenericTeamId(BarrackTeamId);
		NewMinion->FinishSpawning(SpawnTransform);
		NewMinion->SetGoal(Goal);
		MinionPool.Add(NewMinion);
	}
}

void AMinionBarrack::SpawnNewGroup()
{
	int  i = MinionPerGroup;
	while (i > 0)
	{
		FTransform SpawnTransform =GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnIndex())
		{
			SpawnTransform = NextSpawnSpot->GetTransform();
		}
		AMinion* NextMinion = GetNextValueMinion();
		if (!NextMinion) break;;

		SetActorTransform(SpawnTransform);
		NextMinion->Activate();
		--i;
	}
	SpawnMinions(i); 
}
