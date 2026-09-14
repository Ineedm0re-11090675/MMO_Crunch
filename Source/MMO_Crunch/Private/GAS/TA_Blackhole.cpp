#include "TA_Blackhole.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
ATA_Blackhole::ATA_Blackhole()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	DetectionSphereComp = CreateDefaultSubobject<USphereComponent>("Sphere Comp");
	DetectionSphereComp->SetupAttachment(RootComponent);
	DetectionSphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectionSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DetectionSphereComp->OnComponentBeginOverlap.AddDynamic(this,&ATA_Blackhole::ActorInBlackhole);
	DetectionSphereComp->OnComponentEndOverlap.AddDynamic(this,&ATA_Blackhole::ActorLeftBlackhole);

	bReplicates = true;
	ShouldProduceTargetDataOnServer = true;
	PrimaryActorTick.bCanEverTick = true;

	VFXComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VFX Comp"));
	VFXComp->SetupAttachment(RootComponent);
	
}

void ATA_Blackhole::ConfigureBlackhole(float InBlackholeRange, float InPullSpeed, float InBlackholeDuration,
	const FGenericTeamId& InTeamId)
{
	PullSpeed = InPullSpeed;
	DetectionSphereComp->SetSphereRadius(InBlackholeRange);
	SetGenericTeamId(InTeamId);
	BlackholeDuration = InBlackholeDuration;
	BlackholeRange = InBlackholeRange;
}

void ATA_Blackhole::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	TeamID = InTeamID;
}

void ATA_Blackhole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATA_Blackhole, TeamID)
	DOREPLIFETIME_CONDITION_NOTIFY(ATA_Blackhole,BlackholeRange,COND_None,REPNOTIFY_Always);
}

void ATA_Blackhole::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(BlackholeDurationTimerHandle,this,&ATA_Blackhole::StopBlackhole,BlackholeDuration);
		
	}
}

void ATA_Blackhole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		for (TPair<AActor*, UNiagaraComponent*>& Pair : ActorInRangeMap)
		{
			AActor* Target = Pair.Key;
			UNiagaraComponent* NiagaraComponent = Pair.Value;

			FVector PullDir = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();

			Target->SetActorLocation(Target->GetActorLocation() + PullDir * PullSpeed * DeltaTime);
			if (NiagaraComponent)
			{
				NiagaraComponent->SetVariablePosition(BlackholeVFXOriginVariableName,VFXComp->GetComponentLocation());
			}
		}
	}
}

void ATA_Blackhole::ConfirmTargetingAndContinue()
{
	StopBlackhole();
}

void ATA_Blackhole::CancelTargeting()
{
	StopBlackhole();
	Super::CancelTargeting();
}

void ATA_Blackhole::OnRep_BlackholeRangeUpdated()
{
	DetectionSphereComp->SetSphereRadius(BlackholeRange);
}

void ATA_Blackhole::ActorInBlackhole(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryAddTarget(OtherActor);
}

void ATA_Blackhole::ActorLeftBlackhole(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	RemoveTarget(OtherActor); 
}

void ATA_Blackhole::TryAddTarget(AActor* OtherActor)
{
	if (!OtherActor || ActorInRangeMap.Contains(OtherActor)) return;

	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile) return;

	UNiagaraComponent* NiagaraComponent = nullptr;
	if(BlackholeLinkVFX)
	{
		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BlackholeLinkVFX,
			OtherActor->GetRootComponent(),
			NAME_None,
			FVector::Zero(),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false);
		if (NiagaraComponent)
		{
			NiagaraComponent->SetVariablePosition(BlackholeVFXOriginVariableName,VFXComp->GetComponentLocation());
		}
	}
	ActorInRangeMap.Add(OtherActor,NiagaraComponent);
}

void ATA_Blackhole::RemoveTarget(AActor* OtherActor)
{
	if (!OtherActor) return;

	if (ActorInRangeMap.Contains(OtherActor))
	{
		UNiagaraComponent* NiagaraComponent;
		ActorInRangeMap.RemoveAndCopyValue(OtherActor,NiagaraComponent);
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->DestroyComponent();
		}
	}
}

void ATA_Blackhole::StopBlackhole()
{
	TArray<TWeakObjectPtr<AActor>> FinalActors;
	for (TPair<AActor*, UNiagaraComponent*>& Pair : ActorInRangeMap)
	{
		FinalActors.Add(Pair.Key);
		UNiagaraComponent* NiagaraComponent = Pair.Value;
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->DestroyComponent();
		}
	}
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	FGameplayAbilityTargetData_ActorArray* TargetDataArray = new FGameplayAbilityTargetData_ActorArray;
	TargetDataArray->SetActors(FinalActors);

	TargetDataHandle.Add(TargetDataArray);

	FGameplayAbilityTargetData_SingleTargetHit* BlowupLocation = new FGameplayAbilityTargetData_SingleTargetHit; 
	BlowupLocation->HitResult.ImpactPoint = GetActorLocation();

	TargetDataHandle.Add(BlowupLocation);

	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}


