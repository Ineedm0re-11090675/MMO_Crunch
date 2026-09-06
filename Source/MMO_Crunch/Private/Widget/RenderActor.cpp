#include "RenderActor.h"
#include "Components/SceneCaptureComponent2D.h"
ARenderActor::ARenderActor()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	SetRootComponent(RootComponent);

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("CaptureComponent");
	CaptureComponent->SetupAttachment(RootComponent);

	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->FOVAngle =  30.f;
}

void ARenderActor::SetRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	CaptureComponent->TextureTarget = RenderTarget;
}

void ARenderActor::UpdateRender()
{
	if (CaptureComponent)
	{
		CaptureComponent->CaptureScene();
	}
}

void ARenderActor::BeginPlay()
{
	Super::BeginPlay(); 
	//仅渲染拥有者Actor
	CaptureComponent->ShowOnlyActorComponents(this);
	SetActorLocation(FVector{0.f,0.f,100000.f});
}
