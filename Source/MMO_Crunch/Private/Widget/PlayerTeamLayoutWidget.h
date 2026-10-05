#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTeamLayoutWidget.generated.h"


struct FPlayerSelection;

UCLASS()
class UPlayerTeamLayoutWidget: public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections);
private:
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float PlayerTeamWidgetSlotMargin = 5.f;
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TSubclassOf<class UPlayerTeamSlotWidget> PlayerTeamSlotWidgetClass;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* TeamOneLayoutBox;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* TeamTwoLayoutBox;

	UPROPERTY()
	TArray< UPlayerTeamSlotWidget*> TeamSlotWidgets;
};
