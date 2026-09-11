#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameplayMenu.generated.h"

UCLASS()
class UGameplayMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	FOnButtonClickedEvent& GetResumeButtonClickedEventDelegate();
	void SetTitleText(const FString& NewTitle);
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MenuTitle;
	UPROPERTY(meta = (BindWidget))
	class UButton* ResumeButton;
	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton;
	UPROPERTY(meta = (BindWidget))
	UButton* GameQuitButton;

	UFUNCTION()
	void BackToMainMenu();
	UFUNCTION()
	void QuitGame();
};	
	 