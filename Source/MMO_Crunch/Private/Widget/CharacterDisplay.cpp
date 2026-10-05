#include "CharacterDisplay.h"
#include "Camera/CameraComponent.h"
#include "Character/PA_CharacterDefinition.h"
#include "Components/SceneComponent.h"

ACharacterDisplay::ACharacterDisplay()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(FName("RootComponent")));

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
}

void ACharacterDisplay::ConfigureWithCharacterDefinition(const UPA_CharacterDefinition* CharacterDefinition)
{
	if (!CharacterDefinition) return;

	MeshComponent->SetSkeletalMesh(CharacterDefinition->LoadDisplayMesh());
	MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	MeshComponent->SetAnimInstanceClass(CharacterDefinition->LoadDisplayAnimBP());
}
