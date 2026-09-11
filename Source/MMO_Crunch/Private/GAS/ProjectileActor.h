#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"


UCLASS()
class AProjectileActor : public AActor , public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	AProjectileActor();
	void ShootProjectile(
		float InSpeed,
		float InMaxDistance,
		const AActor* InTarget,
		FGenericTeamId InTeamId,
		FGameplayEffectSpecHandle InHitEffectHandle
	);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId);

	virtual FGenericTeamId GetGenericTeamId() const {return TeamId;}

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
private:
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Cue")
	FGameplayTag HitGameplayTag;
	 
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	UPROPERTY(Replicated)
	FVector MoveDir;

	UPROPERTY(Replicated)
	float ProjectileSpeed;

	UPROPERTY()
	const AActor* TargetActor;

	UPROPERTY()
	FGameplayEffectSpecHandle HitEffectHandle;

	FTimerHandle ShootTimerHandle;
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
private:
	void SendLocalGameplayCue(AActor* CueTargetActor,const FHitResult& HitResult);
	void TravelMaxDistanceReached();
};
