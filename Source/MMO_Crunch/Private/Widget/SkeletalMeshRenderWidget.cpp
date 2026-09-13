#include "SkeletalMeshRenderWidget.h"
#include "Components/SceneCaptureComponent2D.h"
#include "RenderActorTargetInterface.h"
#include "SkeletalMeshRenderActor.h"
#include "GameFramework/Character.h"


void USkeletalMeshRenderWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ACharacter* PlayerCharacter = GetOwningPlayerPawn<ACharacter>();
	IRenderActorTargetInterface* PlayerCharacterRenderActorTargetInterface = Cast<IRenderActorTargetInterface>(PlayerCharacter);
	
	if (PlayerCharacter && SkeletalMeshRenderActor)
	{
		SkeletalMeshRenderActor->ConfigureSkeletalMesh(PlayerCharacter->GetMesh()->GetSkeletalMeshAsset(),PlayerCharacter->GetMesh()->GetAnimClass());
		USceneCaptureComponent2D* SceneCaptureComponent = SkeletalMeshRenderActor->GetCaptureComponent();
		if (SceneCaptureComponent && PlayerCharacterRenderActorTargetInterface)
		{
			SceneCaptureComponent->SetRelativeLocation(PlayerCharacterRenderActorTargetInterface->GetCaptureLocalPosition());
			SceneCaptureComponent->SetRelativeRotation(PlayerCharacterRenderActorTargetInterface->GetCaptureLocalRotation());
		}
	}
}

void USkeletalMeshRenderWidget::SpawnRenderActor()
{
	if (!SkeletalMeshRenderActorClass) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SkeletalMeshRenderActor= World->SpawnActor<ASkeletalMeshRenderActor>(SkeletalMeshRenderActorClass,SpawnParams);
}

ARenderActor* USkeletalMeshRenderWidget::GetRenderActor() const
{
	return SkeletalMeshRenderActor;
}
