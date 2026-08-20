#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SendGameplayEvent.generated.h"

UCLASS()
class UAN_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	// Codex: Lesson 36 - Sends the notify's gameplay tag to the owning actor's ASC.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTag EventTag;
};
