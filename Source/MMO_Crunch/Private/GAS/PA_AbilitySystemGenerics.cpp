#include "PA_AbilitySystemGenerics.h"

const FRealCurve* UPA_AbilitySystemGenerics::GetExpCurve() const
{
	return ExperienceCurveTable->FindCurve(ExperienceRowName,"");
}
