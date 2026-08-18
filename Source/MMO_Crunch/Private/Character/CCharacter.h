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
public:
	ACCharacter();

	void ServerSideInit();
	void ClientSideInit();
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
};
