#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TreeNodeInterface.generated.h"


UINTERFACE()
class UTreeNodeInterface : public UInterface
{
	GENERATED_BODY()
};

class ITreeNodeInterface
{
	GENERATED_BODY()

public:
	//只要一个类声称自己是 ITreeNodeInterface，它就必须提供这些能力：拿到自己的 Widget、输入节点、输出节点、对应的数据对象。

	// =0 :我这里只规定必须有这个函数，但是我不给实现，具体实现交给子类。子类必须实现 
	virtual UUserWidget* GetWidget() const = 0;
	virtual TArray<const ITreeNodeInterface*> GetInputs() const = 0 ;
	virtual TArray<const ITreeNodeInterface*> GetOutputs() const = 0 ;
	virtual const UObject* GetItemObject() const = 0;
};
