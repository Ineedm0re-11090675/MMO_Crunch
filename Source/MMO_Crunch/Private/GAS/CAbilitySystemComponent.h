#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CGameplayAbilityTypes.h"
#include "CAbilitySystemComponent.generated.h"

UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UCAbilitySystemComponent();
	void ServerSideInit();
	void InitializeBaseAttributes();
	bool bIsMaxLevel() const ;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpgradeAbilityWithID(ECAbilityInputId InputID);
	UFUNCTION(CLient, Reliable)
	void Client_AbilitySpecLevelUpdated(FGameplayAbilitySpecHandle Handle,int NewLevel);
	
	const TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>>& GetAbility() const ; 
private:

	void ApplyInitialEffects();
	void GiveInitialAbility();
	
	// Codex: Health/Mana threshold tags are now maintained by UCharacterStateComponent.
	void UpdateExperience(const FOnAttributeChangeData& ChangedData);
	

	/*
	 *特殊技能，习得技能
	 */
	
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Ability")
	TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>> GameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Ability")
	TMap<ECAbilityInputId,TSubclassOf<UGameplayAbility>> BasicGameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Ability")
	class UPA_AbilitySystemGenerics* AbilitySystemGenerics ;
};
