#include "ItemTreeWidget.h"

#include "SplineWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"


void UItemTreeWidget::DrawFromNode(const ITreeNodeInterface* NodeInterface)
{
	if (!NodeInterface) return;
	if (CurrentCenterItem == NodeInterface->GetItemObject()) return;

	ClearTree();

	CurrentCenterItem = NodeInterface->GetItemObject();
	float NextLeafXPos =0.f;
	UCanvasPanelSlot* CenterWidgetPanelSlot = nullptr;
	UUserWidget* CenterWidget = CreateWidgetForNode(NodeInterface,CenterWidgetPanelSlot);
	TArray<UCanvasPanelSlot*> LowerStreamSlots, UpperStreamSlots;

	DrawStream(false,NodeInterface,CenterWidget,CenterWidgetPanelSlot,0,NextLeafXPos,LowerStreamSlots);

	float LowerStreamMax = NextLeafXPos - NodeGap.X - NodeSize.X;
	float LowerMoveAmt = 0.f -LowerStreamMax/2.0f;
	for (UCanvasPanelSlot* StreamSlot : LowerStreamSlots)
	{
		StreamSlot->SetPosition(StreamSlot->GetPosition() + FVector2D{LowerMoveAmt,0.f});
	}
	
	NextLeafXPos = 0.f;
	DrawStream(true,NodeInterface,CenterWidget,CenterWidgetPanelSlot,0,NextLeafXPos,UpperStreamSlots);

	float UpperStreamMax = NextLeafXPos - NodeGap.X - NodeSize.X;
	float UpperMoveAmt = 0.f -UpperStreamMax/2.0f;
	for (UCanvasPanelSlot* StreamSlot : UpperStreamSlots)
	{
		StreamSlot->SetPosition(StreamSlot->GetPosition() + FVector2D{UpperMoveAmt,0.f});
	}

	
	CenterWidgetPanelSlot->SetPosition(FVector2D::Zero());
}

void UItemTreeWidget::DrawStream(bool bUpperStream, const ITreeNodeInterface* StartingNodeInterface,
                                 UUserWidget* StartingNodeWidget,UCanvasPanelSlot* StartingNodeSlot, int StartingNodeDepth,
                                 float& NextLeaveXPosition, TArray<UCanvasPanelSlot*>& OutStreamSlots)
{
	//UpperStream : 判断是根的上节点还是下节点，来决定之后Y位置的*-1 or *1
	/*
	 *本质树状递归，类似dfs，根据子节点决定父节点x位置
	 */
	TArray<const ITreeNodeInterface*> NextTreeNodeInterfaces = bUpperStream ? StartingNodeInterface->GetInputs() : StartingNodeInterface->GetOutputs();
	float StartingNodeYPos = (NodeSize.Y + NodeGap.Y) * StartingNodeDepth * (bUpperStream ? -1.0f : 1.0f);
	if (NextTreeNodeInterfaces.Num() == 0)
	{
		//叶节点
		StartingNodeSlot->SetPosition(FVector2D{NextLeaveXPosition,StartingNodeYPos});
		NextLeaveXPosition += NodeGap.X + NodeSize.X;
		return;
	}
	float NextNodePosSum = 0;
	for (const ITreeNodeInterface* NextTreeNodeInterface : NextTreeNodeInterfaces)
	{
		UCanvasPanelSlot* NextWidgetSlot;
		UUserWidget* NextWidget = CreateWidgetForNode(NextTreeNodeInterface,NextWidgetSlot);
		OutStreamSlots.Add(NextWidgetSlot);
		if (bUpperStream)
		{
			CreateConnection(NextWidget,StartingNodeWidget);
		}else
		{
			CreateConnection(StartingNodeWidget,NextWidget);
		}
		DrawStream(bUpperStream,NextTreeNodeInterface,NextWidget,NextWidgetSlot,StartingNodeDepth+1,NextLeaveXPosition,OutStreamSlots);
		NextNodePosSum += NextWidgetSlot->GetPosition().X;
	}

	float StartingNodeXPos = NextNodePosSum / NextTreeNodeInterfaces.Num();
	StartingNodeSlot->SetPosition(FVector2D(StartingNodeXPos,StartingNodeYPos));
}

void UItemTreeWidget::ClearTree()
{
	RootPanel->ClearChildren();
}

UUserWidget* UItemTreeWidget::CreateWidgetForNode(const ITreeNodeInterface* Node, UCanvasPanelSlot*& OutCanvasSlot)
{
	if (!Node) return nullptr;

	UUserWidget* NodeWidget = Node->GetWidget();
	OutCanvasSlot = RootPanel->AddChildToCanvas(NodeWidget);
	if (OutCanvasSlot)
	{
		OutCanvasSlot->SetSize(NodeSize);
		OutCanvasSlot->SetAnchors(FAnchors(0.5f));
		//Alignment 控制的是：Widget 自己的哪个点，放到计算出来的位置上。
		OutCanvasSlot->SetAlignment(FVector2D(0.5f));
		//ZOder就是序列大小，可以把widget分上下
		OutCanvasSlot->SetZOrder(1);
	}
	return NodeWidget;
}

void UItemTreeWidget::CreateConnection(const UUserWidget* From, UUserWidget* To)
{
	if (!From || !To) return;

	USplineWidget* Connection = CreateWidget<USplineWidget>(GetOwningPlayer());
	UCanvasPanelSlot* ConnectionPanelSlot = RootPanel->AddChildToCanvas(Connection);
	if (ConnectionPanelSlot)
	{
		ConnectionPanelSlot->SetAnchors(FAnchors(0.f));
		ConnectionPanelSlot->SetAlignment(FVector2D(0.f));
		ConnectionPanelSlot->SetPosition(FVector2D::Zero());
		ConnectionPanelSlot->SetZOrder(0);
	}
	Connection->SetupSpline(From,To,SourcePortLocalPos,DestinationPortLocalPos,SourcePortDirection,DestinationPortDirection);
	Connection->SetSplineStyle(ConnectionColor,ConnectionThickness);
}
