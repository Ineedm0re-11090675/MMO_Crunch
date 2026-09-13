#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "CrossHairWidget.generated.h"

struct  FGameplayEventData;


UCLASS()
class UCrossHairWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	UPROPERTY(EditDefaultsOnly,Category = "view")
	FLinearColor HasTargetColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly,Category = "view")
	FLinearColor NoTargetColor = FLinearColor::White;
	
	UPROPERTY(meta =(BindWidget))
	class UImage* CrossHairImage;

	void CrossHairTagUpdate(const FGameplayTag Tag,int32 NewCount);

	UPROPERTY()
	class UCanvasPanelSlot* CrossHairCanvasPanelSlot;

	UPROPERTY()
	APlayerController* CachedPlayerController;

	void UpdateCrossHairPosition();

	UPROPERTY()
	const AActor* AimTarget;

	void TargetUpdated(const FGameplayEventData* Data); 
};
