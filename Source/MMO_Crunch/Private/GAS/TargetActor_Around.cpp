#include "TargetActor_Around.h"

#include "CAbilitySystemStatics.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

ATargetActor_Around::ATargetActor_Around()
{
	RootComp = CreateDefaultSubobject<USceneComponent>("RootComponent");
	SetRootComponent(RootComp);

	DetectionSphere = CreateDefaultSubobject<USphereComponent>("Detect Sphere");
	DetectionSphere->SetupAttachment(RootComp);

	DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this,&ATargetActor_Around::ActorInDetectionRange);

	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ATargetActor_Around::ConfigureDetection(float DetectionRadius, const FGenericTeamId& InTeamId,
	const FGameplayTag& InLocalGameplayCueTag)
{
	TargetDetectionRadius = DetectionRadius;
	TeamId = InTeamId;
	LocalGameplayCueTag = InLocalGameplayCueTag;
}

void ATargetActor_Around::ActorInDetectionRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	AActor* AvatarActor = nullptr;
	//Target会找到绑定的Target
	if (OwningAbility)
	{
		AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	}
	if (AvatarActor == OtherActor)
	{
		return;
	}
	if (OtherActor == this)
	{
		return;
	}
	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
	{
		return;
	}

	if (HasAuthority())
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;
		FGameplayAbilityTargetData_ActorArray* ActorArray = new FGameplayAbilityTargetData_ActorArray;
		ActorArray->SetActors(TArray<TWeakObjectPtr<AActor>>{OtherActor});

		TargetDataHandle.Add(ActorArray);
		TargetDataReadyDelegate.Broadcast(TargetDataHandle);
	}

	FHitResult HitResult;
	HitResult.ImpactPoint =OtherActor->GetActorLocation();
	HitResult.ImpactNormal = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	UCAbilitySystemStatics::SendLocalGameplayCue(OtherActor,HitResult,LocalGameplayCueTag);
}

void ATargetActor_Around::OnRep_TargetDetectionRadiusReplicated()
{
	DetectionSphere->SetSphereRadius(TargetDetectionRadius);
}


void ATargetActor_Around::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

void ATargetActor_Around::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATargetActor_Around, TeamId);
	DOREPLIFETIME(ATargetActor_Around, LocalGameplayCueTag);
	DOREPLIFETIME(ATargetActor_Around,TargetDetectionRadius);
	
}
