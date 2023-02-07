#pragma once

#include "AutomataInterface.generated.h"

UINTERFACE()
class UAutomata : public UInterface
{
	GENERATED_BODY()
};

class IAutomata
{
	GENERATED_BODY()

public:

	DECLARE_EVENT_OneParam(IAutomata, SendDisplayData, const TArray<float>&)

	virtual void StepComplete() {}
	virtual void BroadcastData() {}
	virtual void StartNewStep() {}

	virtual void SetBroadcast(SendDisplayData Event) {}

};