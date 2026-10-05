#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterDisplay.generated.h"

class UPA_CharacterDefinition;

UCLASS()
class ACharacterDisplay : public AActor
{
	GENERATED_BODY()
public:
	ACharacterDisplay();

	void ConfigureWithCharacterDefinition(const UPA_CharacterDefinition* CharacterDefinition);
	
private:
	UPROPERTY(EditDefaultsOnly,Category = "Character Display")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly,Category = "Character Display")
	class UCameraComponent* CameraComponent;
	
};
