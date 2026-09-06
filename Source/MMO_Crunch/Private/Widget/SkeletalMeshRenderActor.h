#pragma once

#include "CoreMinimal.h"
#include "RenderActor.h"
#include "SkeletalMeshRenderActor.generated.h"

UCLASS()
class ASkeletalMeshRenderActor : public ARenderActor
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	ASkeletalMeshRenderActor();
	void ConfigureSkeletalMesh( USkeletalMesh* MeshAsset, TSubclassOf<UAnimInstance> AnimBlueprint);
private:
	UPROPERTY(VisibleDefaultsOnly,Category = "Skeletal Mesh Render")
	USkeletalMeshComponent* MeshComponent;
};
