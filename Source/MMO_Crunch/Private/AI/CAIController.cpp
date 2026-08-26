#include "CAIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CCharacter.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ACAIController::ACAIController()
{
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightComp = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightComp->DetectionByAffiliation.bDetectEnemies = true;
	SightComp->DetectionByAffiliation.bDetectFriendlies =false;
	SightComp->DetectionByAffiliation.bDetectNeutrals = false;

	SightComp->SightRadius = 1000.f;
	SightComp->LoseSightRadius = 1200.f;

	SightComp->SetMaxAge(5.f);
	SightComp->PeripheralVisionAngleDegrees = 180.f;

	PerceptionComp->ConfigureSense(*SightComp);
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this,&ACAIController::TargetPerceptionUpdated);
	PerceptionComp->OnTargetPerceptionForgotten.AddDynamic(this,&ACAIController::TargetPerceptionForgottenUpdate);
}

void ACAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(InPawn);
	if (PawnTeamInterface)
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());
		//因为pool的原因，除旧迎新
		ClearAndDisableAllSenses();
		EnableAllSenses();
	}

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InPawn);
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeathStatsAbilityTag()).AddUObject(this,&ACAIController::OnPawnDeathUpdated);
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatsAbilityTag()).AddUObject(this,&ACAIController::OnPawnStunUpdated);
	}
	
}

void ACAIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);
}

void ACAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		if (!GetCurrentTargetActor())
		{
			SetCurrentTargetActor(TargetActor);
		}
	}else
	{
		ForgetActorIfDead(TargetActor); 
	}
}

void ACAIController::TargetPerceptionForgottenUpdate(AActor* ForgottenActor)
{
	if (!ForgottenActor) return;

	if (ForgottenActor == GetCurrentTargetActor())
	{
		SetCurrentTargetActor(GetNextTarget() );
	}
	
}

const UObject* ACAIController::GetCurrentTargetActor() const
{
	const UBlackboardComponent* Blackboard  = GetBlackboardComponent();
	if(Blackboard)
	{
		return GetBlackboardComponent()->GetValueAsObject(TargetBlackboardKeyName);
	}
	return nullptr;
}

void ACAIController::SetCurrentTargetActor(AActor* NewTarget)
{
	UBlackboardComponent* Blackboard  = GetBlackboardComponent();
	if (Blackboard == nullptr)  return;
	if (NewTarget)
	{
		Blackboard->SetValueAsObject(TargetBlackboardKeyName,NewTarget);
	}else
	{
		Blackboard->ClearValue(TargetBlackboardKeyName);
	}
	
}

AActor* ACAIController::GetNextTarget() const
{
	if (PerceptionComp)
	{
		TArray<AActor*> FoundActors;
		PerceptionComp->GetPerceivedHostileActors(FoundActors);
		if (FoundActors.Num() != 0)
		{
			return FoundActors[0];
		}
	}
	return nullptr;
}

void ACAIController::ForgetActorIfDead(AActor* ActorToForget)
{
	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC) return;

	if (ActorASC->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeathStatsAbilityTag()))
	{
		 for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iterator = PerceptionComp->GetPerceptualDataIterator();Iterator;++Iterator)
		 {
			 if (Iterator.Key() != ActorToForget)
			 {
				 continue;
			 }

		 	for (FAIStimulus& Stimulus : Iterator.Value().LastSensedStimuli)
		 	{
		 		//虽然在Character死亡是UnRegister了，但是刺激源还没过期，所以这里调成Max，变成过期
		 		Stimulus.SetStimulusAge(TNumericLimits<float>::Max());
		 	}
		 }
	}
}

void ACAIController::ClearAndDisableAllSenses()
{
	if (PerceptionComp)
	{
		PerceptionComp->AgeStimuli(TNumericLimits<float>::Max());
		for (auto SenseConfig =PerceptionComp->GetSensesConfigIterator();SenseConfig;++SenseConfig)
		{
			PerceptionComp->SetSenseEnabled((*SenseConfig)->GetSenseImplementation(),false);
		}
	}
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(TargetBlackboardKeyName);
	}
}

void ACAIController::EnableAllSenses()
{
	if (PerceptionComp)
	{
		for (auto SenseConfig =PerceptionComp->GetSensesConfigIterator();SenseConfig;++SenseConfig)
		{
			PerceptionComp->SetSenseEnabled((*SenseConfig)->GetSenseImplementation(),true);
		}
	}
}

void ACAIController::OnPawnDeathUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		//Stop Tree
		GetBrainComponent()->StopLogic("Dead");
		ClearAndDisableAllSenses();
		bIsPawnDead = true;
	}else
	{
		GetBrainComponent()->StartLogic();
		EnableAllSenses();
		bIsPawnDead = false;
	}
}

void ACAIController::OnPawnStunUpdated(const FGameplayTag Tag, int32 Count)
{
	if(bIsPawnDead) return;
	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("Stun");
	}else
	{
		GetBrainComponent()->StartLogic();
	}
}
