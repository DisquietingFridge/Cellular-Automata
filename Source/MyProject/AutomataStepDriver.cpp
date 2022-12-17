#include "AutomataStepDriver.h"

DECLARE_EVENT(UAutomataStepDriver, DriverStepEvent)

void UAutomataStepDriver::TimerFired()
{
	WaitAndFinalize.Broadcast();
	UpdateAndNewStep.Broadcast();
}

void UAutomataStepDriver::SetTimer(float StepPeriod)
{
	GetWorld()->GetTimerManager().SetTimer(StepTimer, this, &UAutomataStepDriver::TimerFired, StepPeriod, true);
}

void UAutomataStepDriver::SetEvents(DriverStepEvent FinishStep, DriverStepEvent NextStep)
{
	WaitAndFinalize = FinishStep;
	UpdateAndNewStep = NextStep;
}