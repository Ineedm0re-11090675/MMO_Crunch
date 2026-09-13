#include "TargetActor_Line.h"
#include "../MMO_Crunch.h"
#include "NiagaraComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ATargetActor_Line::ATargetActor_Line()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComp);

	TargetEndDetectionSphere = CreateDefaultSubobject<USphereComponent>("Target End Detection Sphere");
	TargetEndDetectionSphere->SetupAttachment(RootComp);
	TargetEndDetectionSphere->SetCollisionResponseToChannel(ECC_SpringArm,ECR_Ignore);

	LazerVFX = CreateDefaultSubobject<UNiagaraComponent>("Lazer VFX");
	LazerVFX->SetupAttachment(RootComp );

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	ShouldProduceTargetDataOnServer = true;
	
	AvatarActor = nullptr;
}

void ATargetActor_Line::ConfigureTargetSetting(float NewTargetRange, float NewDetectionCylinderRadius,
	float NewTargetingInterval, FGenericTeamId NewTargetingTeamId, bool bShouldDrawDebug)
{
	TargetRange = NewTargetRange;
	DetectionCylinderRadius = NewDetectionCylinderRadius;
	TargetingInterval = NewTargetingInterval;
	SetGenericTeamId(NewTargetingTeamId);
	bDrawDebug = bShouldDrawDebug;
}

void ATargetActor_Line::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	TargetingTeamId = TeamID;
}

void ATargetActor_Line::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATargetActor_Line, TargetingTeamId);
	DOREPLIFETIME(ATargetActor_Line, TargetRange);
	DOREPLIFETIME(ATargetActor_Line, DetectionCylinderRadius);
	DOREPLIFETIME(ATargetActor_Line, AvatarActor);
}

void ATargetActor_Line::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	if (!OwningAbility)
	{
		return;
	}
	AvatarActor = OwningAbility->GetOwningActorFromActorInfo();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(PeriodicalTargetingTimerHandle, this,&ATargetActor_Line::DoTargetCheckAndReport,TargetingInterval,true);
	}
}

void ATargetActor_Line::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTargetTrack();
}

void ATargetActor_Line::BeginDestroy()
{
	if (GetWorld() && PeriodicalTargetingTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(PeriodicalTargetingTimerHandle);
	}
	Super::BeginDestroy(); 
}

void ATargetActor_Line::DoTargetCheckAndReport()
{
	if (!HasAuthority()) return;
	
	TSet<AActor*> OverlappingActorSet;
	TargetEndDetectionSphere->GetOverlappingActors(OverlappingActorSet);

	TArray<TWeakObjectPtr<AActor>> OverlappingActors;
	for (AActor* OverlappingActor : OverlappingActorSet)
	{
		if (ShouldReportActorAsTarget(OverlappingActor))
		{
			OverlappingActors.Add(OverlappingActor);
		}
	}
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	FGameplayAbilityTargetData_ActorArray* ActorArray = new FGameplayAbilityTargetData_ActorArray;
	ActorArray->SetActors(OverlappingActors);

	TargetDataHandle.Add(ActorArray);
	//此class父类 自带的，会在GA_Lazer处callback
	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}

void ATargetActor_Line::UpdateTargetTrack()
{
	FVector TargetLocation = GetActorLocation();
	FRotator TargetRotation = GetActorRotation();
	if (AvatarActor)
	{
		AvatarActor->GetActorEyesViewPoint(TargetLocation, TargetRotation);
	}
	FVector LookEndLoc = TargetLocation + TargetRotation.Vector()*100000;
	FRotator LookEndRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookEndLoc);
	SetActorRotation(LookEndRot);

	FVector SweepEndLoc = GetActorLocation() + LookEndRot.Vector()*TargetRange;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(AvatarActor);

	FCollisionResponseParams ObjectParams(ECR_Overlap);
	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByChannel(
		HitResults,
		GetActorLocation(),
		SweepEndLoc,
		FQuat::Identity,
		ECC_WorldDynamic,
		FCollisionShape::MakeSphere(DetectionCylinderRadius),
		QueryParams,
		ObjectParams
		);
	FVector LineEndLoc = SweepEndLoc;
	float LazerLength = TargetRange;
	for (const FHitResult& HitResult : HitResults)
	{
		if (HitResult.GetActor())
		{
			if (GetTeamAttitudeTowards(*HitResult.GetActor()) != ETeamAttitude::Friendly)
			{
				LineEndLoc= HitResult.ImpactPoint;
				LazerLength = FVector::Distance(LineEndLoc, GetActorLocation());
				break;
			}
		}
	}
	TargetEndDetectionSphere->SetWorldLocation(LineEndLoc);
	if (LazerVFX)
	{
		LazerVFX->SetVariableFloat(LazerFXLengthParaName, LazerLength/100);
	}
}

bool ATargetActor_Line::ShouldReportActorAsTarget(const AActor* ActorToCheck) const
{
	if (!ActorToCheck) return false;
	if (ActorToCheck == AvatarActor) return false;
	if (ActorToCheck == this ) return false;
	if (GetTeamAttitudeTowards(*ActorToCheck) != ETeamAttitude::Hostile) return false;
	return true; 
}
