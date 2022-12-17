#pragma once

#include "AutomataStepDriver.generated.h"

UCLASS()
class MYPROJECT_API UAutomataStepDriver : public UObject
{
	GENERATED_BODY()
	

	public:
	DECLARE_EVENT(UAutomataStepDriver, DriverStepEvent)
	void SetEvents(DriverStepEvent FinishStepEvent, DriverStepEvent UpdateDisplayEvent);
	void SetTimer(float StepPeriod);

	private:
	// This will make the ruleset wait for step to finish calculating and finalize switch data
	DriverStepEvent WaitAndFinalize;
	// This event will apply switch data to display and kick off new step
	DriverStepEvent UpdateAndNewStep;

	FTimerHandle StepTimer;

	void TimerFired();

	

};
