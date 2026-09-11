#include "GameplayMenu.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UGameplayMenu::NativeConstruct()
{
	Super::NativeConstruct();
	MainMenuButton->OnClicked.AddDynamic(this,&UGameplayMenu::BackToMainMenu);
	GameQuitButton->OnClicked.AddDynamic(this,&UGameplayMenu::QuitGame);
}

FOnButtonClickedEvent& UGameplayMenu::GetResumeButtonClickedEventDelegate()
{
	return ResumeButton->OnClicked;
}

void UGameplayMenu::SetTitleText(const FString& NewTitle)
{
	MenuTitle->SetText(FText::FromString(NewTitle));
}

void UGameplayMenu::BackToMainMenu()
{
}

void UGameplayMenu::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,true)  ;
}
