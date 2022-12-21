#include "AutomataStepDriver.h"

DECLARE_EVENT(UAutomataStepDriver, DriverStepEvent)

void UAutomataStepDriver::TimerFired()
{
	WaitAndFinalize.Broadcast();
	UpdateDisplay.Broadcast();
	StartNewStep.Broadcast();
}

void UAutomataStepDriver::SetTimer(float StepPeriod)
{
	GetWorld()->GetTimerManager().SetTimer(StepTimer, this, &UAutomataStepDriver::TimerFired, StepPeriod, true);
}

void UAutomataStepDriver::SetEvents(DriverStepEvent Finalize, DriverStepEvent Update, DriverStepEvent NextStep)
{
	WaitAndFinalize = Finalize;
	UpdateDisplay = Update;
	StartNewStep = NextStep;
}