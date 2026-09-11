#include "MatchStatWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "FrameWork/StormCore.h"
#include "Kismet/GameplayStatics.h"

void UMatchStatWidget::NativeConstruct()
{ 
	Super::NativeConstruct();
	StormCore = Cast<AStormCore>(UGameplayStatics::GetActorOfClass(this, AStormCore::StaticClass()));
	if (StormCore )
	{
		StormCore->OnInfluencerCount.AddUObject(this,&UMatchStatWidget::UpdateTeamInfluence);
		StormCore->OnGoalReached.AddUObject(this,&UMatchStatWidget::MatchFinished);
		GetWorld()->GetTimerManager().SetTimer(UpdateProgressTimerHandle,this,&UMatchStatWidget::UpdateProgress,ProgressUpdateInterval,true);
	}
}
 
void UMatchStatWidget::UpdateTeamInfluence(int TeamOneCount, int TeamTwoCount)
{
	TeamOneCountText->SetText(FText::AsNumber(TeamOneCount));
	TeamTwoCountText->SetText(FText::AsNumber(TeamTwoCount));
}

void UMatchStatWidget::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	float Progress = WinningTeam == 0?1.f:0.f;
	GetWorld()->GetTimerManager().ClearTimer(UpdateProgressTimerHandle);
	ProgressImage->GetDynamicMaterial()->SetScalarParameterValue(ProgressDynamicMaterialParaName, Progress);
}

void UMatchStatWidget::UpdateProgress()
{
	if (StormCore)
	{
		float Progress = StormCore->GetProgress();
		ProgressImage->GetDynamicMaterial()->SetScalarParameterValue(ProgressDynamicMaterialParaName, Progress);
	}
}
