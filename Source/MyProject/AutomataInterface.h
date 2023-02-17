#pragma once

#include "AutomataInterface.generated.h"

class UAutomataDisplay;

// contains members common to virtually all automata
USTRUCT()
struct FBaseAutomataStruct
{
	GENERATED_BODY()

public:

	// describes each cell's neighbors
	TArray<TArray<int>> Neighborhoods;

	// Display that the automata writes relevant information to each step.
	UAutomataDisplay* Display = nullptr;

	// records what step the cells were switched to an "off" position,
	// used to give fade-out effect for dead cells
	// a future step denotes that the cell is on
	TArray<float> SwitchStepBuffer;

	// simulation step. 
	//Used to record switches made during next state transition in SwitchStepBuffer
	float NextStep = 0;

	// state of each cell in the grid.
	TArray<int> CurrentStates;

	FBaseAutomataStruct() {}

	FBaseAutomataStruct(TArray<TArray<int>> NewNeighborhoods, UAutomataDisplay* NewDisplay)
	{
		Neighborhoods = NewNeighborhoods;
		Display = NewDisplay;

		int NumCells = Neighborhoods.Num();
		SwitchStepBuffer.Init(TNumericLimits<int32>::Min(), NumCells);
		CurrentStates.Init(0, NumCells);
	}
};

UINTERFACE()
class UAutomata : public UInterface
{
	GENERATED_BODY()
};

//struct FBaseAutomataStruct;

class IAutomata
{
	GENERATED_BODY()

public:

	virtual void SetNeighborhoods(TArray<TArray<int>> Neighbs) {}

	virtual void StepComplete() {}
	virtual void BroadcastData() {}
	virtual void StartNewStep() {}

	virtual void SetBaseMembers(FBaseAutomataStruct NewBaseMembers) {}

};



