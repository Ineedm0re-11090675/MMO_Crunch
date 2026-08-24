#include "CGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId NewTeamId =GetTeamIDForPlayerController(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(NewTeamId); 
	}
	NewPlayerController->StartSpot = FindNextStartSpotForTeam(NewTeamId);

	return NewPlayerController;
}

FGenericTeamId ACGameMode::GetTeamIDForPlayerController(const APlayerController* PC) const
{
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
