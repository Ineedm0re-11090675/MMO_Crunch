#include "CrossHairWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "GAS/CAbilitySystemStatics.h" 
void UCrossHairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CrossHairImage->SetVisibility(ESlateVisibility::Hidden);

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetCrossHairTag()).AddUObject(this, &UCrossHairWidget::CrossHairTagUpdate);
		OwnerASC->GenericGameplayEventCallbacks.Add(UCAbilitySystemStatics::GetTargetUpdatedTag()).AddUObject(this, &UCrossHairWidget::TargetUpdated);
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
	if (!AimTarget)
	{
		FVector2D ViewPortSize = FVector2D((float)SizeX,(float)SizeY);
		CrossHairCanvasPanelSlot->SetPosition(ViewPortSize/2.f / ViewPortScale);
		return;
	}
	FVector2D TargetScreenPosition;
	CachedPlayerController->ProjectWorldLocationToScreen(AimTarget->GetActorLocation(), TargetScreenPosition);
	if (TargetScreenPosition.X>0 && TargetScreenPosition.X<SizeX && TargetScreenPosition.Y>0 && TargetScreenPosition.Y<SizeY)
	{
		CrossHairCanvasPanelSlot->SetPosition(TargetScreenPosition/ ViewPortScale);
	}
}

void UCrossHairWidget::TargetUpdated(const FGameplayEventData* Data)
{
	AimTarget = Data->Target;
	CrossHairImage->SetColorAndOpacity(AimTarget?HasTargetColor:NoTargetColor);
	
}
