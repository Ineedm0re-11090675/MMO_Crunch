#include "CNetStatic.h"
#include "OnlineSubsystemUtils.h"

IOnlineSessionPtr UCNetStatic::GetSessionPtr(const UObject* WorldContextObject)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	if (Subsystem)
	{
		return Subsystem->GetSessionInterface();
	}
	return nullptr;
}

IOnlineIdentityPtr UCNetStatic::GetIdentityPtr(const UObject* WorldContextObject)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	if (Subsystem)
	{
		return Subsystem->GetIdentityInterface();
	}
	return nullptr;
}

FOnlineSessionSettings UCNetStatic::GenerateOnlineSessionSettings(const FName& SessionName,
                                                                  const FString& SessionSearchId, int Port)
{
	FOnlineSessionSettings OnlineSessionSettings;
	OnlineSessionSettings.bIsLANMatch = false;
	OnlineSessionSettings.bShouldAdvertise =true;
	OnlineSessionSettings.NumPublicConnections =GetPlayerCountPerTeam() *2;
	OnlineSessionSettings.bUsesPresence = false;
	OnlineSessionSettings.bAllowJoinViaPresence = false;
	OnlineSessionSettings.bAllowJoinInProgress =false;
	OnlineSessionSettings.bAllowInvites =true;
	OnlineSessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	OnlineSessionSettings.bUseLobbiesIfAvailable =false;
	OnlineSessionSettings.bUsesStats =true;


	OnlineSessionSettings.Set(GetSessionNameKey(),SessionName.ToString(),EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionSettings.Set(GetSessionSearchIdKey(),SessionSearchId,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionSettings.Set(GetPortKey(),Port,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	return OnlineSessionSettings; 
}

uint8 UCNetStatic::GetPlayerCountPerTeam()
{
	return 5;
}

bool UCNetStatic::IsSessionServer(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetNetMode() == NM_DedicatedServer;
}

FName UCNetStatic::GetSessionNameKey()
{
	return FName("SESSION_NAME");
}

FString UCNetStatic::GetSessionNameStr()
{
	return GetCommandLineArgAsString(GetSessionNameKey());
}

FString UCNetStatic::GetSessionSearchIdStr()
{
	return GetCommandLineArgAsString(GetSessionSearchIdKey());
}

FName UCNetStatic::GetSessionSearchIdKey()
{
	return FName("SESSION_SEARCH_ID");
}

int UCNetStatic::GetSessionPort()
{
	return GetCommandLineArgAsInt(GetPortKey());
}

FName UCNetStatic::GetPortKey()
{
	return FName("PORT");
}

FString UCNetStatic::GetCommandLineArgAsString(const FName& ParaName)
{
	FString OutVal = "";
	FString CommandLineArg = FString::Printf(TEXT("%s="),*(ParaName).ToString());

	FParse::Value(FCommandLine::Get(),*CommandLineArg,OutVal);
	return OutVal;
}

int UCNetStatic::GetCommandLineArgAsInt(const FName& ParaName)
{
	int OutVal = 0;
	FString CommandLineArg = FString::Printf(TEXT("%s="),*(ParaName).ToString());

	FParse::Value(FCommandLine::Get(),*CommandLineArg,OutVal);
	return OutVal;
}
