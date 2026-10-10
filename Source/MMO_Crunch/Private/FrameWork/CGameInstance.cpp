#include "CGameInstance.h"

#include "Network/CNetStatic.h"

void UCGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode() == NM_DedicatedServer || GetWorld()->GetNetMode() == NM_ListenServer)
	{
		LoadLevelAndListen(GameLevel);
	}
}

void UCGameInstance::Init()
{
	Super::Init();
	if(GetWorld()->IsEditorWorld()) return;

	if (UCNetStatic::IsSessionServer(this))
	{
		CreateSession();
	}
}

void UCGameInstance::CreateSession()
{
	IOnlineSessionPtr SessionPtr = UCNetStatic::GetSessionPtr(this);
	
	
	ServerSessionName = UCNetStatic::GetSessionNameStr();
	FString ServerSessionSearchID = UCNetStatic::GetSessionSearchIdStr();
	int ServerSessionPort = UCNetStatic::GetSessionPort();

	UE_LOG(LogTemp, Warning, TEXT("###Server Session Port With Name : %s , SearchId : %s ,Port : %d"), *(ServerSessionName),*(ServerSessionSearchID),ServerSessionPort);
} 

void UCGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	const FName LevelURL = FName(*FPackageName::ObjectPathToObjectName(Level.ToString()));

	if (LevelURL != "")
	{
		GetWorld()->ServerTravel(LevelURL.ToString() + "?listen");
	}
}
