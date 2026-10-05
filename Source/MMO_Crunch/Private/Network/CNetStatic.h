#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CNetStatic.generated.h"

UCLASS()
class UCNetStatic : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static uint8 GetPlayerCountPerTeam();
};
