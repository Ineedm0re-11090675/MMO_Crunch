#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelWidget.generated.h"

struct FOnAttributeChangeData;

UCLASS()
class ULevelWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(EditDefaultsOnly,Category ="Visual")
	FName PercentMaterialParaName = "Percent";
	
	UPROPERTY( meta  = (BindWidget))
	class UImage* LevelProgress;
	UPROPERTY( meta  = (BindWidget))
	class UTextBlock* Level;

	FNumberFormattingOptions NumberFormatOptions;

	UPROPERTY()
	const class UAbilitySystemComponent* OwnASC;

	void UpdateGauge(const FOnAttributeChangeData& Data);
};
