#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlineSessionSettings.h"
#include "CNetStatic.generated.h"

UCLASS()
class UCNetStatic : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static IOnlineSessionPtr GetSessionPtr(const UObject* WorldContextObject);
	static IOnlineIdentityPtr GetIdentityPtr(const UObject* WorldContextObject);
	
	
	static FOnlineSessionSettings GenerateOnlineSessionSettings(const FName& SessionName,const FString& SessionSearchId,int Port);
	static uint8 GetPlayerCountPerTeam();

	static bool IsSessionServer(const UObject* WorldContextObject);


	static FName GetSessionNameKey();
	static FString GetSessionNameStr();

	
	static FString GetSessionSearchIdStr();
	static FName GetSessionSearchIdKey();

	static int GetSessionPort();
	static FName GetPortKey();

	static FString GetCommandLineArgAsString(const FName& ParaName);

	static int GetCommandLineArgAsInt(const FName& ParaName);
};
