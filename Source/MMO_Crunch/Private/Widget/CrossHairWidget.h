#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "CrossHairWidget.generated.h"

UCLASS()
class UCrossHairWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	UPROPERTY(meta =(BindWidget))
	class UImage* CrossHairImage;

	void CrossHairTagUpdate(const FGameplayTag Tag,int32 NewCount);

	UPROPERTY()
	class UCanvasPanelSlot* CrossHairCanvasPanelSlot;

	UPROPERTY()
	APlayerController* CachedPlayerController;

	void UpdateCrossHairPosition();
};
