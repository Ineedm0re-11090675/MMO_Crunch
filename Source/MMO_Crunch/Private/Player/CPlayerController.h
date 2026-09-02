#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

UCLASS()
class ACPlayerController :public APlayerController,public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	// Only Called On Server
	void OnPossess(APawn* NewPawn) override;
	// Only Called On Client, also Listener server
	void AcknowledgePossession(APawn* NewPawn) override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupInputComponent() override;
private:
	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<class UGameplayWidget> GamePlayWidgetClass;

	UPROPERTY()
	UGameplayWidget* GameplayWidget;

	void SpawnGameplayWidget();

	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	UPROPERTY(EditDefaultsOnly , Category = "Input")
	class UInputMappingContext* UIInputMapping;

	UPROPERTY(EditDefaultsOnly , Category = "Input")
	class UInputAction* ShopToggleInputAction;

	UFUNCTION()
	void ToggleShop();
};