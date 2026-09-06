#pragma once

#include "CoreMinimal.h"
#include "RenderActorWidget.h"
#include "SkeletalMeshRenderWidget.generated.h"

UCLASS()
class USkeletalMeshRenderWidget : public URenderActorWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
private:
	virtual void SpawnRenderActor() override;
	virtual ARenderActor* GetRenderActor() const override;
	UPROPERTY(EditDefaultsOnly,Category = "Skeletal Mesh Render")
	TSubclassOf<class ASkeletalMeshRenderActor> SkeletalMeshRenderActorClass;

	UPROPERTY()
	ASkeletalMeshRenderActor* SkeletalMeshRenderActor;
};
