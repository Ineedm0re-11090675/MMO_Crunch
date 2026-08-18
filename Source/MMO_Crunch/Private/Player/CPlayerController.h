#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

UCLASS()
class ACPlayerController :public APlayerController
{
	GENERATED_BODY()
public:
	// Only Called On Server
	void OnPossess(APawn* NewPawn) override;
	// Only Called On Client, also Listener server
	void AcknowledgePossession(APawn* NewPawn) override;

private:
	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;
};