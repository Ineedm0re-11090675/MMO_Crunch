#include "TA_Blackhole.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

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

void ATA_Blackhole::OnRep_BlackholeRangeUpdated()
{
	DetectionSphereComp->SetSphereRadius(BlackholeRange);
}

void ATA_Blackhole::ActorInBlackhole(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void ATA_Blackhole::ActorLeftBlackhole(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}


