#include "TargetActor_GroundPick.h"
#include "Components/DecalComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/OverlapResult.h"
#include "MMO_Crunch/MMO_Crunch.h"

void ATargetActor_GroundPick::SetTargetAreaRadius(float NewTargetAreaRadius)
{
	TargetAreaRadius = NewTargetAreaRadius;
	DecalComponent->DecalSize = FVector{NewTargetAreaRadius};
}

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("RootComponent"));
	
	DecalComponent = CreateDefaultSubobject<UDecalComponent>("Decal Component");
	DecalComponent->SetupAttachment(GetRootComponent());
}

void ATargetActor_GroundPick::ConfirmTargetingAndContinue()
{
	TArray<FOverlapResult> OverlapResults;

	FCollisionObjectQueryParams CollisionParams;
	CollisionParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(TargetAreaRadius);
	GetWorld()->OverlapMultiByObjectType(OverlapResults,GetActorLocation(),FQuat::Identity,CollisionParams,CollisionShape);

	TSet<AActor*> TargetActors;

	IGenericTeamAgentInterface* OwnerTeamInterface = nullptr;
	 if (OwningAbility)
	 {
	 	OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	 }
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamInterface &&
			OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Friendly &&
			 !bShouldTargetFriendly)
		{
			continue;
		}
		if (OwnerTeamInterface &&
			OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Hostile &&
			 !bShouldTargetEnemy)
		{
			continue;
		}
		TargetActors.Add(OverlapResult.GetActor());
	}
	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(),false);

	FGameplayAbilityTargetData_SingleTargetHit* HitLoc = new FGameplayAbilityTargetData_SingleTargetHit();
	HitLoc->HitResult.ImpactPoint= GetActorLocation();

	//把target中心传给GroundBlast；
	TargetData.Add(HitLoc);
	
	TargetDataReadyDelegate.Broadcast(TargetData);
}

void ATargetActor_GroundPick::SetTargetOption(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetEnemy= bTargetEnemy;
	bShouldTargetFriendly = bTargetFriendly;
}

void ATargetActor_GroundPick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		SetActorLocation(GetTargetPoint());
	}
}

FVector ATargetActor_GroundPick::GetTargetPoint() const
{
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController()) return GetActorLocation();

	FHitResult Hit;

	FVector ViewLocation;
	FRotator ViewRotation;

	PrimaryPC->GetPlayerViewPoint(ViewLocation,ViewRotation);

	FVector ViewEnd = ViewLocation + ViewRotation.Vector() * TargetTraceRange;
	GetWorld()->LineTraceSingleByChannel(Hit,ViewLocation,ViewEnd,ECollisionChannel::ECC_Target );

	if (!Hit.bBlockingHit)
	{
		GetWorld()->LineTraceSingleByChannel(Hit,ViewEnd,ViewEnd+FVector::DownVector * TNumericLimits<float>::Max(),ECollisionChannel::ECC_Target);
	}
	if (!Hit.bBlockingHit)
	{
		return GetActorLocation();
	}

	if (bShouldDrawDebug)
	{
		DrawDebugSphere(GetWorld(),Hit.ImpactPoint,TargetAreaRadius,32,FColor::Red); 
	}
	return Hit.ImpactPoint;
}
