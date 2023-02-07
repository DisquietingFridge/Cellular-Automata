#pragma once

#include "AutomataStepDriver.generated.h"

class IAutomata;

UCLASS()
class MYPROJECT_API UAutomataStepDriver : public UObject
{
	GENERATED_BODY()
	

	public:

	void SetAutomata(IAutomata* newAutomata);
	void SetTimer(float StepPeriod);

	private:

	FTimerHandle StepTimer;

	IAutomata* Automata;

	void TimerFired();

	

};
