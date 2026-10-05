#include "CGameMode.h"
#include "EngineUtils.h"
#include "StormCore.h"
#include "GameFramework/PlayerStart.h"
#include "Player/CPlayerController.h"
#include "Player/CPlayerState.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId NewTeamId =GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(NewTeamId); 
	}
	NewPlayerController->StartSpot = FindNextStartSpotForTeam(NewTeamId);

	return NewPlayerController;
}

void ACGameMode::StartPlay()
{
	Super::StartPlay();
	AStormCore* StormCore = GetStormCore();
	if (StormCore)
	{
		StormCore->OnGoalReached.AddUObject(this,&ACGameMode::MatchFinished);
	}
}

UClass* ACGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	ACPlayerState* CPlayerState = InController->GetPlayerState<ACPlayerState>();
	if (CPlayerState && CPlayerState->GetSelectedPawnClass())
	{
		return CPlayerState->GetSelectedPawnClass();
	}
	return BackupPawn;
}

APawn* ACGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayer);	
	FGenericTeamId TeamId = GetTeamIDForPlayer(NewPlayer);

	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}

	StartSpot = FindNextStartSpotForTeam(TeamId);
	NewPlayer->StartSpot = StartSpot;
	
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

FGenericTeamId ACGameMode::GetTeamIDForPlayer(const AController* PC) const
{
	ACPlayerState* CPlayerState = PC->GetPlayerState<ACPlayerState>();
	if (CPlayerState && CPlayerState->GetTeamIDBasedOnSlot())
	{
		return CPlayerState->GetTeamIDBasedOnSlot();
	}
	static int PlayerCount =0;
	PlayerCount++;
	return FGenericTeamId(PlayerCount%2);
}

AActor* ACGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamId) const
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamId);
	if (!StartSpotTag) return nullptr;
	UWorld* World = GetWorld();

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag =FName("Taken");
			return *It;
		}
	}
	return nullptr;
}

void ACGameMode::MatchFinished(AActor* ViewTarget , int WinningTeam)
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ACPlayerController> It(World); It; ++It)
		{
			It->MatchFinished(ViewTarget,WinningTeam);
		}
	} 
}

class AStormCore* ACGameMode::GetStormCore() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<AStormCore> It(World); It; ++It)
		{
			return *It;
		}
	}
	return nullptr;
}
