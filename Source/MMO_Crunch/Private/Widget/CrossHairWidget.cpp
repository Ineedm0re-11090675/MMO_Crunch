#include "CrossHairWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"

void UCrossHairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CrossHairImage->SetVisibility(ESlateVisibility::Hidden);

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetCrossHairTag()).AddUObject(this, &UCrossHairWidget::CrossHairTagUpdate);
	}
	CachedPlayerController = GetOwningPlayer();
	CrossHairCanvasPanelSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CrossHairCanvasPanelSlot)
	{
		UE_LOG(LogTemp,Error,TEXT("CrossHairCanvasPanelSlot is NULL"));
	}
}

void UCrossHairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (CrossHairImage->GetVisibility() == ESlateVisibility::Visible)
	{
		UpdateCrossHairPosition();
	}
}

void UCrossHairWidget::CrossHairTagUpdate(const FGameplayTag Tag, int32 NewCount)
{
	CrossHairImage->SetVisibility(NewCount > 0 ? ESlateVisibility::Visible: ESlateVisibility::Hidden);
}

void UCrossHairWidget::UpdateCrossHairPosition()
{
	if (!CrossHairCanvasPanelSlot || !CachedPlayerController) return;

	int32 SizeX,SizeY;
	float ViewPortScale = UWidgetLayoutLibrary::GetViewportScale(this);
	CachedPlayerController->GetViewportSize(SizeX,SizeY);
	FVector2D ViewPortSize = FVector2D((float)SizeX,(float)SizeY);
	CrossHairCanvasPanelSlot->SetPosition(ViewPortSize/2.f / ViewPortScale);
}
