#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include  "Blueprint/IUserObjectListEntry.h"
#include "AbilityGauge.generated.h"

USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityClass;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FName AbilityName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FText Description;
};


UCLASS()
class UAbilityGauge : public UUserWidget , public IUserObjectListEntry 
{
	GENERATED_BODY()
public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeConstruct() override;
	void ConfigureAbilityWithWidgetData(const FAbilityWidgetData* WidgetData);
private:
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName IconMaterialParaName =  "Icon" ;

	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName CooldownPercentParaName =  "Percent" ;

	UPROPERTY(EditDefaultsOnly,Category="Cooldown")
	float CooldownUpdateInterval = 0.1f;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownDurationText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CooldownCounterText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CostText;

	UPROPERTY()
	UGameplayAbility* AbilityCDO;//ClassDefaultObject

	void AbilityCommittedCallback(UGameplayAbility* Ability);

	void StartCooldown(float CooldownTimeRemaining,float CooldownDuration);


	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;

	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	FNumberFormattingOptions WholeNumericFormattingOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions; 

	void CooldownFinish();
	void UpdateCooldown();
};
