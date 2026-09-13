#include "StormCore.h"

#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"

AStormCore::AStormCore()
{
	PrimaryActorTick.bCanEverTick = true;
	InfluenceRange = CreateDefaultSubobject<USphereComponent>("InfluenceRange");
	InfluenceRange->SetupAttachment(GetRootComponent());

	InfluenceRange->OnComponentBeginOverlap.AddDynamic(this,&AStormCore::NewInfluenceInRange);
	InfluenceRange->OnComponentEndOverlap.AddDynamic(this,&AStormCore::InfluenceLeftRange);

	ViewCam = CreateDefaultSubobject<UCameraComponent>("ViewCam");
	ViewCam->SetupAttachment(GetRootComponent());
	
	GroundDecalComponent = CreateDefaultSubobject<UDecalComponent>("DecalComponent");
	GroundDecalComponent->SetupAttachment(GetRootComponent());
}

void AStormCore::Tick(float DeltaTime)
{ 
	Super::Tick(DeltaTime);
	if (CoreToCapture)
	{
		FVector CoreMoveDir = (GetMesh()->GetComponentLocation() - CoreToCapture->GetActorLocation()).GetSafeNormal();
		CoreToCapture->AddActorWorldOffset(CoreMoveDir * CoreCaptureSpeed * DeltaTime);
	}
}

void AStormCore::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AStormCore, CoreToCapture,COND_None,REPNOTIFY_Always);
}

void AStormCore::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AStormCore::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AStormCore,InfluenceRange))
	{
		InfluenceRange->SetSphereRadius(InfluenceRadius);
		FVector DecalSize = GroundDecalComponent->DecalSize;
		GroundDecalComponent->DecalSize = FVector{DecalSize.X,InfluenceRadius,InfluenceRadius};
		/*在U的 Decal 里，DecalSize 可以理解成一个投影盒子：
		* X：贴花的投影深度，也就是贴花往物体表面“打进去”多深。
		* Y：贴花平面的一个方向尺寸，可以理解成宽度/半径。
		* Z：贴花平面的另一个方向尺寸，可以理解成高度/半径。
		*/
	}
}

float AStormCore::GetProgress() const
{
	FVector TeamTwoGoalLoc = TeamTwoGoal->GetActorLocation();
	FVector VectorFromTeamOne = GetActorLocation() - TeamTwoGoalLoc;

	VectorFromTeamOne.Z = 0.f;
	return VectorFromTeamOne.Length() / TravelLength;
}

void AStormCore::BeginPlay()
{
	Super::BeginPlay();
	FVector TeamOneGoalLoc = TeamOneGoal->GetActorLocation();
	FVector TeamTwoGoalLoc = TeamTwoGoal->GetActorLocation();
	FVector GoalOffset = TeamOneGoalLoc - TeamTwoGoalLoc;
	GoalOffset.Z = 0.f;

	TravelLength = GoalOffset.Length();
}

void AStormCore::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	OwnAIC = Cast<AAIController>(NewController);
}

void AStormCore::NewInfluenceInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == TeamOneGoal)
	{
		GoalReached(0);
	}
	if (OtherActor == TeamTwoGoal)
	{
		GoalReached(1);
	}
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(OtherActor);
	if (TeamAgentInterface)
	{
		if (TeamAgentInterface->GetGenericTeamId() == 0 )
		{
			TeamOneInfluenceCount++;
		}else if (TeamAgentInterface->GetGenericTeamId() == 1 )
		{
			TeamTwoInfluenceCount++; 
		}
		UpdateTeamWeight();
	}
}

void AStormCore::InfluenceLeftRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(OtherActor);
	if (TeamAgentInterface)
	{
		if (TeamAgentInterface->GetGenericTeamId() == 0 )
		{
			TeamOneInfluenceCount--;
			if (TeamOneInfluenceCount < 0)
			{
				TeamOneInfluenceCount=0;
			}
		}else if (TeamAgentInterface->GetGenericTeamId() == 1 )
		{
			TeamTwoInfluenceCount--;
			if (TeamTwoInfluenceCount < 0)
			{
				TeamTwoInfluenceCount=0;
			}
		}
		UpdateTeamWeight();
	}
}

void AStormCore::UpdateTeamWeight()
{
	OnInfluencerCount.Broadcast(TeamOneInfluenceCount,TeamTwoInfluenceCount);
	if (TeamOneInfluenceCount == TeamTwoInfluenceCount)
	{
		TeamWeight =0;
	}else
	{
		float TeamOffset = TeamOneInfluenceCount - TeamTwoInfluenceCount;
		float TeamTotal = TeamOneInfluenceCount + TeamTwoInfluenceCount;

		TeamWeight = TeamOffset / TeamTotal;
	}
	UpdateGoal();
	UE_LOG(LogTemp, Warning, TEXT("Team Weight: %f"), TeamWeight);
}

void AStormCore::UpdateGoal()
{
	if (!HasAuthority()) return;

	if (!OwnAIC) return;

	if (!GetCharacterMovement()) return;

	if (TeamWeight > 0 )
	{
		OwnAIC->MoveToActor(TeamOneGoal);
	}
	else
	{
 	OwnAIC->MoveToActor(TeamTwoGoal);
	}
	float Speed = MaxMoveSpeed * FMath::Abs(TeamWeight);

	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AStormCore::OnRep_CoreToCapture()
{
	if (CoreToCapture)
	{
		CaptureCore();
	}
}

void AStormCore::GoalReached(int WinningTeam)
{
	OnGoalReached.Broadcast(this, WinningTeam);

	if (!HasAuthority())
	{
		return;
	}

	MaxMoveSpeed = 0.f;
	CoreToCapture = WinningTeam ==0 ? TeamOneCore:TeamTwoCore;
	CaptureCore();
}

void AStormCore::ExpandFinished()
{
	CoreToCapture->SetActorLocation(GetMesh()->GetComponentLocation());
	CoreToCapture->AttachToComponent(GetMesh(),FAttachmentTransformRules::KeepRelativeTransform,"root");
	
	GetMesh()->GetAnimInstance()->Montage_Play(CaptureMontage);
}

void AStormCore::CaptureCore()
{
	float ExpandDuration = GetMesh()->GetAnimInstance()->Montage_Play(ExpandMontage);
	CoreCaptureSpeed = FVector::Distance(GetMesh()->GetComponentLocation(),CoreToCapture->GetActorLocation()) / ExpandDuration;

	CoreToCapture->SetActorEnableCollision(false);
	GetCharacterMovement()->MaxWalkSpeed = 0.f;

	FTimerHandle ExpandTimerHandle;
	GetWorldTimerManager().SetTimer(ExpandTimerHandle,this,&AStormCore::ExpandFinished,ExpandDuration);
}



