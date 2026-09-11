#include "ProjectileActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Net/UnrealNetwork.h"

AProjectileActor::AProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComp);
}

void AProjectileActor::ShootProjectile(float InSpeed, float InMaxDistance, const AActor* InTarget,
	FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle)
{
	TargetActor = InTarget;
	ProjectileSpeed = InSpeed;
	SetGenericTeamId(InTeamId);

	FRotator OwnerViewRot =GetActorRotation();

	if (GetOwner())
	{
		FVector OwnerViewLoc;
		GetOwner()->GetActorEyesViewPoint(OwnerViewLoc, OwnerViewRot);
	}
	MoveDir = OwnerViewRot.Vector();

	HitEffectHandle = InHitEffectHandle;

	float MaxTravelTime = InMaxDistance /InSpeed;

	GetWorld()->GetTimerManager().SetTimer(ShootTimerHandle,this,&AProjectileActor::TravelMaxDistanceReached,MaxTravelTime);
}

void AProjectileActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectileActor,MoveDir)
	DOREPLIFETIME(AProjectileActor,TeamId)
	DOREPLIFETIME(AProjectileActor,ProjectileSpeed)
	
}

void AProjectileActor::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	TeamId = NewTeamId;
}

void AProjectileActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!OtherActor || OtherActor == GetOwner()) return;
	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile) return;

	UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (IsValid(OtherASC))
	{
		if (HasAuthority() &&  HitEffectHandle.IsValid())
		{
			OtherASC->ApplyGameplayEffectSpecToSelf(*HitEffectHandle.Data.Get());
			GetWorldTimerManager().ClearTimer(ShootTimerHandle);
		}
		FHitResult HitResult;
		HitResult.Location = GetActorLocation();
		HitResult.Normal =GetActorForwardVector();

		SendLocalGameplayCue(OtherActor,HitResult);
		Destroy();
	}
}


void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		if (TargetActor)
		{
			MoveDir = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		}
	}
	SetActorLocation(GetActorLocation() + MoveDir * ProjectileSpeed * DeltaTime);
}

void AProjectileActor::SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult)
{
	FGameplayCueParameters Parameters;
	Parameters.Location = HitResult.Location;
	Parameters.Normal = HitResult.Normal;

	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(CueTargetActor,HitGameplayTag,EGameplayCueEvent::Executed,Parameters);
}

void AProjectileActor::TravelMaxDistanceReached()
{
	Destroy();
}
