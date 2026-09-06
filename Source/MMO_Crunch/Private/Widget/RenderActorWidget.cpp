#include "RenderActorWidget.h"
#include "RenderActor.h" 
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SizeBox.h"

void URenderActorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RenderSizeBox->SetWidthOverride(RenderSize.X);
	RenderSizeBox->SetHeightOverride(RenderSize.Y); 
}

void URenderActorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SpawnRenderActor();
	ConfigureRenderActor();
	BeginRenderCapture();
}

void URenderActorWidget::BeginDestroy()
{
	StopRenderCapture();
	Super::BeginDestroy(); 
}

void URenderActorWidget::ConfigureRenderActor()
{
	if (!GetRenderActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("No RenderActor Spawn, Nothing will be render"));
		return;
	}

	//完善配置
	RenderTarget = NewObject<UTextureRenderTarget2D>(this );
	RenderTarget->InitAutoFormat((int)RenderSize.X, (int)RenderSize.Y);
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8_SRGB;
	GetRenderActor()->SetRenderTarget(RenderTarget);

	//设置配置
	UMaterialInstanceDynamic* DynamicMaterial = DisplayImage->GetDynamicMaterial();
	if (DynamicMaterial) 
	{
		DynamicMaterial->SetTextureParameterValue(DisplayImageRenderTargetParaName, RenderTarget);
	}
}

void URenderActorWidget::BeginRenderCapture()
{
	if (RenderFrameRate <= 0)
	{
		return;
	}
	
	RenderTickInterval = 1.f / static_cast<float>(RenderFrameRate);
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(RenderTimerHandle,this,&URenderActorWidget::UpdateRender,RenderTickInterval,true);
		 
	}
}

void URenderActorWidget::UpdateRender()
{
	if (GetRenderActor())
	{
		GetRenderActor()->UpdateRender();
	}
}

void URenderActorWidget::StopRenderCapture()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(RenderTimerHandle); 
	}
}
