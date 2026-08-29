#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_AbilitySystemGenerics.generated.h"

class UGameplayEffect;
class UGameplayAbility;

UCLASS()
class UPA_AbilitySystemGenerics: public UDataAsset
{
	GENERATED_BODY()
public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const{ return DeathEffect; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStatsEffect() const{ return FullStatsEffect; }
	FORCEINLINE const TArray<TSubclassOf<UGameplayEffect>>& GetInitialEffect() const{ return InitialEffect; }
	FORCEINLINE const TArray<TSubclassOf<UGameplayAbility>>&  GetPassiveAbility() const{ return PassiveAbility; }
	FORCEINLINE const UDataTable* GetBaseStatsDataTable() const{ return BaseStatsDataTable; }
	const FRealCurve* GetExpCurve() const ;
private:
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatsEffect;

	
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffect;

	UPROPERTY(EditDefaultsOnly,Category = "Base Stats")
	UDataTable* BaseStatsDataTable;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbility;


	UPROPERTY(EditDefaultsOnly,Category = "Level")
	FName ExperienceRowName = "ExperienceNeededToReachLevel";

	UPROPERTY(EditDefaultsOnly,Category = "Level")
	class UCurveTable* ExperienceCurveTable;
};
