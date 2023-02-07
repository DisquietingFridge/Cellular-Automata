#pragma once

#include "AutomataInterface.h"
#include "Rulesets.generated.h"


UCLASS()
class ULifelikeRule : public UObject, public IAutomata
{
	GENERATED_BODY()


	// records what step the cells were switched to an "off" position,
	// used to give fade-out effect for dead cells
	// a future time denotes that the cell is on
	TArray<float> SwitchStepBuffer;

	// simulation step. 
	//Used to record switches made during next state transition in SwitchStepBuffer
	float NextStep;

	//Set that stores the birth rules for the automata
	TArray<bool> BirthRules;
	//Set that stores the survival rules for the automata
	TArray<bool> SurviveRules;

	// states of the grid, whether a cell is alive
	TArray<bool> CurrentStates;
	TArray<bool> NextStates;

	// Flags for each cell, describing whether they require evaluation
	TArray<bool> EvalFlaggedThisStep;
	TArray<bool> EvalFlaggedLastStep;

	// describes each cell's neighbors
	TArray<TArray<int>> Neighborhoods;

	// for each cell, describes the cells that have it as a neighbor
	// only different from Neighborhoods in asymmetric neighborhoods
	TArray<TArray<int>> NeighborsOf;

	// responsible for calculating the next step asynchronously
	TFuture<void> AsyncState;


	void CalcStepSwitches();

	void ApplyCellRules();

	void TimestepPropertyShift();

	int GetCellAliveNeighbors(int CellID) const;

	void PostNeighborhoodSetup();

	SendDisplayData SwitchStepsReady;


	public:

	void InitializeCellStates(float Probability);
	void InitializeCellRules(FString BirthString, FString SurviveString);

	// many arrays depend on the number of cells,
	// which is described by the Neighborhood array
	

	void SetNeighborhoods(TArray<TArray<int>> Neighbs) override;

	void SetNeighborsOf(TArray<TArray<int>> NeighbsOf)
	{
		NeighborsOf = NeighbsOf;
	}

	void SetBroadcast(SendDisplayData Event) override;

	void StepComplete() override;
	void BroadcastData() override;
	void StartNewStep() override;
};

UCLASS()
class UAntRule : public UObject, public IAutomata
{
	GENERATED_BODY()

	// records what step the cells were switched to an "off" position,
	// used to give fade-out effect for dead cells
	// a future time denotes that the cell is on
	TArray<float> SwitchStepBuffer;

	// simulation step. 
	//Used to record switches made during next state transition in SwitchStepBuffer
	float NextStep;

	// describes state of each cell, influences ant based on CellSequence
	// Assumes that ants change cells in turn, not simultaneously
	TArray<int> CurrentStates;

	// describes each cell's neighbors
	// these should be ordered in clockwise cardinal direction for consistent rotation behavior
	TArray<TArray<int>> Neighborhoods;

	// Ruleset that determines the tile's effect on the ant, e.g. {right, left} would be {1, -1}, or {1,3}
	TArray<int> CellSequence = { 1, -1 };

	// for each ant, describes its orientation: which neighbor it will move to to change its position
	TArray<int> AntOrientations = { 0 };

	// describes each ant's position, used to query cell states and neighborhoods
	TArray<int> AntPositions = { 0 };

	// responsible for calculating the next step asynchronously
	TFuture<void> AsyncState;

	SendDisplayData SwitchStepsReady;

	void MoveAnts();

	void PostNeighborhoodSetup();

public:

	void SetNeighborhoods(TArray<TArray<int>> Neighbs) override;

	void InitializeAnts(int NumAnts);

	void StepComplete() override;
	void BroadcastData() override;
	void StartNewStep() override;

	void SetBroadcast(SendDisplayData Event) override;

};

namespace AutomataFuncs {
	static void MakeNeighborsOf(TArray<TArray<int>>& NeighborsOf, TArray<TArray<int>>& Neighborhoods);
}
