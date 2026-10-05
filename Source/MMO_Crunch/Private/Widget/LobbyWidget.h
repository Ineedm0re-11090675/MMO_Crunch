#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

UCLASS()
class ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MainSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	UWidget* TeamSelectionRoot;

	UPROPERTY(meta = (BindWidget))
	UWidget* HeroSelectionRoot;

	UPROPERTY(meta = (BindWidget))
	class UTileView* CharacterSelectionTileView;
	
	UPROPERTY(meta=(BindWidget))
	class UButton* StartHeroSelectionButton;

	UPROPERTY(meta=(BindWidget))
	UButton* StartMatchButton;
	
	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* TeamSelectionSlotGridPanel;

	UPROPERTY(meta = (BindWidget))
	class UPlayerTeamLayoutWidget* PlayerTeamLayoutWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Team Selection")
	TSubclassOf<class UTeamSelectionWidget> TeamSelectionWidgetClass;

	UPROPERTY()
	TArray<UTeamSelectionWidget*> TeamSelectionSlots;

	UPROPERTY(meta = (BindWidget))
	class UAbilityListView* AbilityListView;

	void ClearAndPopulateTeamSelectionSlots();
	void SelectSlot(uint8 NewSlotId);

	UPROPERTY()
	class  ALobbyPlayerController* LobbyPlayerController;

	void ConfigureGameState();

	UPROPERTY()
	class ACGameState* CGameState;

	FTimerHandle ConfigureGameStateTimerHandle;

	void UpdatePlayerSelectionDisplay(const TArray<struct FPlayerSelection>& PlayerSelections); 

	UPROPERTY()
	class ACPlayerState* CPlayerState;

	UFUNCTION()
	void StartHeroSelectionButtonClicked();
	UFUNCTION()
	void StartMatchButtonClicked();

	void SwitchToHeroSelection();

	void CharacterDefinitionLoaded();

	void CharacterSelected(UObject* SelectedUObject);

	UPROPERTY(EditDefaultsOnly, Category = "Character Display")
	TSubclassOf<class ACharacterDisplay> CharacterDisplayClass;

	UPROPERTY()
	ACharacterDisplay* CharacterDisplay;

	void SpawnCharacterDisplay();
	void UpdateCharacterDisplay(const FPlayerSelection& PlayerSelection);
};