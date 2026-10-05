#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamSelectionWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotlicked, uint8 /*SlotID*/);
UCLASS()
class UTeamSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetSlotId(uint8 NewSlotId);

	void UpdateSlotInfo(const FString& PlayerNickName);

	virtual void NativeConstruct() override;
	
	FOnSlotlicked OnSlotlicked;
private:
	UPROPERTY(meta=(BindWidget))
	class UButton* SelectionButton;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* InfoText;

	UFUNCTION()
	void SelectionButtonClicked();

	uint8 SlotID;
	
};
