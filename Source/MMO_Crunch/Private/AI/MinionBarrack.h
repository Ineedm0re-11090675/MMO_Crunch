#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "MinionBarrack.generated.h"

UCLASS()
class AMinionBarrack : public AActor
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;	
private:
	UPROPERTY(EditAnywhere,Category = "Spawn")
	FGenericTeamId BarrackTeamId;
	
	UPROPERTY() 
	TArray<class AMinion*>MinionPool;

	UPROPERTY(EditAnywhere,Category = "Spawn")
	int MinionPerGroup =3;
	UPROPERTY(EditAnywhere,Category = "Spawn")
	float SpawnInterval =6.f;

	UPROPERTY(EditAnywhere,Category = "Spawn")
	AActor* Goal;
	
	UPROPERTY(EditAnywhere,Category = "Spawn")
	TSubclassOf<AMinion> MinionClass;

	UPROPERTY(EditAnywhere,Category = "Spawn")
	TArray<APlayerStart*> SpawnSpots;


	int NextSpawnIndex = -1;

	const APlayerStart* GetNextSpawnIndex();
	AMinion* GetNextValueMinion() const; 
	
	void SpawnMinions(int Amt);

	void SpawnNewGroup();
	FTimerHandle SpawnMinionsIntervalTimerHandle; 
};
