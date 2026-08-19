#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "CCharacter.generated.h"
/*
 **
 */
UCLASS()
class ACCharacter : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;	
public:
	ACCharacter();

	void ServerSideInit();
	void ClientSideInit();

	bool IsLocallyControlledByPlayer() const;

	virtual void PossessedBy(AController* NewController) override;
	/*
	 *Gameplay Ability
	*/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	UPROPERTY(VisibleDefaultsOnly,Category = "Gameplay Effects")
	class UCAbilitySystemComponent* CAbilitySystemComponent;
	UPROPERTY()
	class UCAttributeSet* CAttributeSet;
	/*
	 *UI
	*/
private:
	UPROPERTY(VisibleDefaultsOnly,Category = "UI")
	class UWidgetComponent* OverHeadWidget;

	void ConfigureOverHeadStatsWidget();

	UPROPERTY(EditDefaultsOnly,Category= "UI")
	float  HSGVisibleRangeSquared =10000000.f;
	
	UPROPERTY(EditDefaultsOnly,Category= "UI")
	float  HSGVisibleCheckGap = 1.f;
	
	FTimerHandle OverHeadStatsGaugeUpdateTimer;

	void UpdateOverHeadStatsGauge();
};
