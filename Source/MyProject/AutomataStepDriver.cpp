#include "AutomataStepDriver.h"

#include "AutomataInterface.h"

DECLARE_EVENT(UAutomataStepDriver, DriverStepEvent)

void UAutomataStepDriver::TimerFired()
{
	Automata->StepComplete();
	Automata->BroadcastData();
	Automata->StartNewStep();
}

void UAutomataStepDriver::SetAutomata(IAutomata* newAutomata)
{
	Automata = newAutomata;
}

void UAutomataStepDriver::SetTimer(float StepPeriod)
{
	GetWorld()->GetTimerManager().SetTimer(StepTimer, this, &UAutomataStepDriver::TimerFired, StepPeriod, true);
}