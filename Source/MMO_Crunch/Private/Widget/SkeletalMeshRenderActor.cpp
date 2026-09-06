#include "SkeletalMeshRenderActor.h"

void ASkeletalMeshRenderActor::BeginPlay()
{
	Super::BeginPlay();
	//Owner See;
	MeshComponent->SetVisibleInSceneCaptureOnly(true);
}

ASkeletalMeshRenderActor::ASkeletalMeshRenderActor()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//UE 里大多数灯光默认都在 Lighting Channel 0 
	MeshComponent->SetLightingChannels(false,true,false);
}

void ASkeletalMeshRenderActor::ConfigureSkeletalMesh(USkeletalMesh* MeshAsset,
	TSubclassOf<UAnimInstance> AnimBlueprint)
{
	MeshComponent->SetSkeletalMeshAsset(MeshAsset);
	MeshComponent->SetAnimInstanceClass(AnimBlueprint);
}
