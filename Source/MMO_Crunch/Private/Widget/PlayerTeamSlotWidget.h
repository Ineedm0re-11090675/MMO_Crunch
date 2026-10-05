#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTeamSlotWidget.generated.h"

class UPA_CharacterDefinition;

UCLASS()
class UPlayerTeamSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateSlot(const FString& PlayerName,const UPA_CharacterDefinition* CharacterDefinition);

	virtual void NativeConstruct() override;	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
private:
	UPROPERTY(Transient,meta=(BindWidgetAnim))
	UWidgetAnimation* HoverAnim;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* PlayerCharacterIcon;

	UPROPERTY(EditDefaultsOnly,Category = "Visual")
	FName CharacterIconMatParaName = "Icon";

	UPROPERTY(EditDefaultsOnly,Category = "Visual")
	FName CharacterEmptyNameParaName = "Empty";

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;

	FString CachedPlayerNamePtr;
	FString CachedCharacterNamePtr;

	void UpdateNameText();
};
