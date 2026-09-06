#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RenderActorWidget.generated.h"

class ARenderActor;
//UE自带的纯虚函数
UCLASS(Abstract)
class URenderActorWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void BeginDestroy() override;
public:

	//UE自带的纯虚函数
	//PURE 宏第二参 是默认函数实现,void 可以没有m 
	virtual void SpawnRenderActor() PURE_VIRTUAL(URenderActorWidget::SpawnRenderActor,);
	virtual ARenderActor* GetRenderActor() const PURE_VIRTUAL(URenderActorWidget::GetRenderActor,return nullptr;)

	void ConfigureRenderActor();

	void BeginRenderCapture();
	void UpdateRender();
	void StopRenderCapture();
	
	UPROPERTY(meta = (BindWidget))
	class UImage* DisplayImage;
	
	UPROPERTY(meta = (BindWidget))
	class USizeBox* RenderSizeBox;
	
	UPROPERTY(EditDefaultsOnly,Category = "Render Actor")
	FName DisplayImageRenderTargetParaName = "RenderTarget";

	UPROPERTY(EditDefaultsOnly,Category = "Render Actor")
	FVector2D RenderSize;

	UPROPERTY(EditDefaultsOnly,Category = "Render Actor")
	int RenderFrameRate = 24;

	float RenderTickInterval ;
	FTimerHandle RenderTimerHandle;

	UPROPERTY()
	class UTextureRenderTarget2D* RenderTarget;
};
