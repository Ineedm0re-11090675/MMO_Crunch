#include "CGameMode.h"
#include "EngineUtils.h"
#include "StormCore.h"
#include "GameFramework/PlayerStart.h"
// Codex: Use the owning player controller and defer RestartPlayer by one tick after death completion.
#include "GameFramework/PlayerController.h"
#include "Player/CPlayerController.h"
#include "Player/CPlayerState.h"
#include "TimerManager.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	// Codex: Controller creation can fail; do not dereference a missing controller.
	if (!NewPlayerController) return nullptr;
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId NewTeamId =GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(NewTeamId); 
	}
	// Codex: Choose a team start when spawning the Pawn, not once here and again during RestartPlayer.

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

// Codex: Restart only player controllers on the server, after MapleCombat has detached the dying Pawn.
bool ACGameMode::RequestPlayerRespawn(APlayerController* PlayerController)
{
	if (!HasAuthority() || !IsValid(PlayerController) || !GetWorld())
	{
		return false;
	}

	const TWeakObjectPtr<APlayerController> WeakPlayerController(PlayerController);
	const FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(
		this, &ThisClass::RestartPlayerAfterDeath, WeakPlayerController);
	GetWorldTimerManager().SetTimerForNextTick(RespawnDelegate);
	return true;
}

// Codex: A controller may have been reassigned before the deferred respawn runs; never spawn a duplicate Pawn.
void ACGameMode::RestartPlayerAfterDeath(TWeakObjectPtr<APlayerController> PlayerController)
{
	APlayerController* Controller = PlayerController.Get();
	if (!HasAuthority() || !IsValid(Controller))
	{
		return;
	}

	if (Controller->GetPawn())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Codex] Skipping respawn for %s because it already possesses a Pawn."), *GetNameSafe(Controller));
		return;
	}

	RestartPlayer(Controller);
	if (!Controller->GetPawn())
	{
		UE_LOG(LogTemp, Error, TEXT("[Codex] RestartPlayer did not spawn a Pawn for %s."), *GetNameSafe(Controller));
	}
}

UClass* ACGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	ACPlayerState* CPlayerState = InController->GetPlayerState<ACPlayerState>();
	// Codex: Resolve the selected class once; loading the character class can be synchronous.
	if (CPlayerState)
	{
		const TSubclassOf<APawn> SelectedPawnClass = CPlayerState->GetSelectedPawnClass();
		if (SelectedPawnClass)
		{
			return SelectedPawnClass;
		}
	}
	return BackupPawn;
}

APawn* ACGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Codex: Keep the spawn path safe even if a controller is unavailable during travel.
	if (!NewPlayer) return nullptr;
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayer);	
	FGenericTeamId TeamId = GetTeamIDForPlayer(NewPlayer);

	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}

	// Codex: A missing team tag or PlayerStart must never replace a valid engine-selected start with nullptr.
	if (AActor* TeamStartSpot = FindNextStartSpotForTeam(TeamId))
	{
		StartSpot = TeamStartSpot;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No PlayerStart for team %d; using engine-selected start %s"), TeamId.GetId(), *GetNameSafe(StartSpot));
	}
	if (!IsValid(StartSpot))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot spawn player %s: no valid PlayerStart"), *GetNameSafe(NewPlayer));
		return nullptr;
	}
	NewPlayer->StartSpot = StartSpot;
	
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

FGenericTeamId ACGameMode::GetTeamIDForPlayer(const AController* PC) const
{
	// Codex: A valid slot determines the team; loading a selected Pawn class does not.
	const ACPlayerState* CPlayerState = PC ? PC->GetPlayerState<ACPlayerState>() : nullptr;
	if (CPlayerState)
	{
		const FGenericTeamId SlotTeamId = CPlayerState->GetTeamIDBasedOnSlot();
		if (SlotTeamId.GetId() != FGenericTeamId::NoTeam.GetId())
		{
			return SlotTeamId;
		}
	}
	static int PlayerCount =0;
	PlayerCount++;
	return FGenericTeamId(PlayerCount%2);
}

AActor* ACGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamId)
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamId);
	if (!StartSpotTag) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	// Codex: Rotate among matching starts; changing PlayerStartTag to Taken exhausted them permanently.
	TArray<APlayerStart*> MatchingStarts;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			MatchingStarts.Add(*It);
		}
	}
	if (MatchingStarts.IsEmpty()) return nullptr;
	int32& NextIndex = NextTeamStartIndex.FindOrAdd(TeamId);
	APlayerStart* Result = MatchingStarts[NextIndex % MatchingStarts.Num()];
	++NextIndex;
	return Result;
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
