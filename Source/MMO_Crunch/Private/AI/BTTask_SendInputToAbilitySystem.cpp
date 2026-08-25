#include "BTTask_SendInputToAbilitySystem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAIController.h"

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIC = OwnerComp.GetAIOwner();
	if (OwnerAIC)
	{
		UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIC->GetPawn());
		if (OwnerASC)
		{
			OwnerASC->PressInputID((uint32)InputID);
			return EBTNodeResult::Succeeded; 
		}
	}

	return EBTNodeResult::Failed; 
}
