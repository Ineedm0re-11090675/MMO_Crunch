#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "CharacterEntryWidget.generated.h"

class UPA_CharacterDefinition;

UCLASS()
class UCharacterEntryWidget : public UUserWidget,public IUserObjectListEntry
{
	GENERATED_BODY()
public:

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	FORCEINLINE const UPA_CharacterDefinition* GetCharacterDefinition() const{ return CharacterDefinition;}

	void SetSelected(bool bIsSelected);
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* CharacterIcon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;

	UPROPERTY(EditDefaultsOnly , Category = "Character")
	FName IconTextureMatParaName = "Icon";
	
	UPROPERTY(EditDefaultsOnly , Category = "Character")
	FName SaturationMatParaName = "Saturation";
	
	UPROPERTY()
	const UPA_CharacterDefinition* CharacterDefinition;
};
