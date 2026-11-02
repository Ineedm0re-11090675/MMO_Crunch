#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "CGameMode.generated.h"

// Codex: Forward declare the controller type used by the respawn entry point.
class APlayerController;

UCLASS()
class  ACGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;	
	
	virtual void StartPlay() override;
	// Codex: Centralize player respawn after MapleCombat finishes its death flow.
	bool RequestPlayerRespawn(APlayerController* PlayerController);
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
private:
	// Codex: Defer RestartPlayer until MapleCombat has finished its current death callback stack.
	void RestartPlayerAfterDeath(TWeakObjectPtr<APlayerController> PlayerController);

	FGenericTeamId GetTeamIDForPlayer(const AController* PC) const;

	AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamId);
	// Codex: Rotate through team starts without changing their PlayerStartTag, so later spawns can reuse them.
	TMap<FGenericTeamId, int32> NextTeamStartIndex;
	UPROPERTY(EditDefaultsOnly,Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;

	UPROPERTY(EditDefaultsOnly,Category = "Team")
	TSubclassOf<APawn> BackupPawn;
	
	class AStormCore* GetStormCore() const ;

	void MatchFinished(AActor* ViewTarget , int WinningTeam);
};
