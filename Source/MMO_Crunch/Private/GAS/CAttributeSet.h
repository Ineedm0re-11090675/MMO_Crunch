#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CAttributeSet.generated.h"
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/*
 **
 */
UCLASS()
class UCAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	ATTRIBUTE_ACCESSORS(UCAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UCAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UCAttributeSet, Mana)
	ATTRIBUTE_ACCESSORS(UCAttributeSet, MaxMana)
	ATTRIBUTE_ACCESSORS(UCAttributeSet, AttackDamage)
	ATTRIBUTE_ACCESSORS(UCAttributeSet, Armor)
	ATTRIBUTE_ACCESSORS(UCAttributeSet, MoveSpeed)
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/*
	GE 开始执行
	   ↓
	准备把 Health 改成某个新值
	   ↓
	PreAttributeChange
	   ↓
	真正修改 Health
	   ↓
	GE 执行结束
	   ↓
	PostGameplayEffectExecute

	如果是算Damage，只有Pre会漏，所以Post也要有，兜底
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
private:
	UPROPERTY(ReplicatedUsing=OnRep_Heath)
	FGameplayAttributeData Health;
	UPROPERTY(ReplicatedUsing=OnRep_MaxHeath)
	FGameplayAttributeData MaxHealth;
	UPROPERTY(ReplicatedUsing=OnRep_Mana)
	FGameplayAttributeData Mana;
	UPROPERTY(ReplicatedUsing=OnRep_MaxMana)
	FGameplayAttributeData MaxMana;

	UPROPERTY(ReplicatedUsing=OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;
	UPROPERTY(ReplicatedUsing=OnRep_Armor)
	FGameplayAttributeData Armor;
	UPROPERTY(ReplicatedUsing=OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	UFUNCTION()
	void OnRep_Heath(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxHeath(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
};
