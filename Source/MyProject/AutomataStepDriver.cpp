#include "AutomataStepDriver.h"

#include "AutomataInterface.h"

DECLARE_EVENT(UAutomataStepDriver, DriverStepEvent)

void UAutomataStepDriver::TimerFired()
{
	Automata->StepComplete();
	Automata->BroadcastData();
	Automata->StartNewStep();
}

void UAutomataStepDriver::BeginDestroy()
{
	Super::BeginDestroy();

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Aggggh I'm being destroyed noooo");
}

void UAutomataStepDriver::SetAutomata(IAutomata* newAutomata)
{
	Automata = newAutomata;
}

void UAutomataStepDriver::SetTimer(float StepPeriod)
{
	GetWorld()->GetTimerManager().SetTimer(StepTimer, this, &UAutomataStepDriver::TimerFired, StepPeriod, true);
}