#include "MMC_BaseAttackDamage.h"

#include "CAttributeSet.h"

UMMC_BaseAttackDamage::UMMC_BaseAttackDamage()
{
	DamageCaptureDef.AttributeToCapture = UCAttributeSet::GetAttackDamageAttribute();
	DamageCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	
	ArmorCaptureDef.AttributeToCapture = UCAttributeSet::GetArmorAttribute();
	ArmorCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	RelevantAttributesToCapture.Add(DamageCaptureDef);
	RelevantAttributesToCapture.Add(ArmorCaptureDef);
}

float UMMC_BaseAttackDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();


	//
	//为什么这里要读Tag？Tag可能意味着buff，可能加攻击，加防御，所以得加入到parameters里
	//
	float AttackDamage = 0.f;
	GetCapturedAttributeMagnitude(DamageCaptureDef,Spec,EvaluateParameters,AttackDamage);
	float Armor =0.f;
	GetCapturedAttributeMagnitude(ArmorCaptureDef,Spec,EvaluateParameters,Armor);

	float Damage = AttackDamage* (1 - (Armor)/(Armor +100));

	return -Damage;
}
