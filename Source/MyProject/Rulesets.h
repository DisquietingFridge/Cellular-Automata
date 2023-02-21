#pragma once

#include "AutomataInterface.h"
#include "Rulesets.generated.h"



UCLASS()
class ULifelikeRule : public UObject, public IAutomata
{
	GENERATED_BODY()

	FBaseAutomataStruct BaseMembers;

	//Set that stores the birth rules for the automata
	TArray<bool> BirthRules;
	//Set that stores the survival rules for the automata
	TArray<bool> SurviveRules;

	TArray<int> NextStates;

	// Flags for each cell, describing whether they require evaluation
	TArray<bool> EvalFlaggedThisStep;
	TArray<bool> EvalFlaggedLastStep;

	// for each cell, describes the cells that have it as a neighbor
	// only different from Neighborhoods in asymmetric neighborhoods
	TArray<TArray<int>> NeighborsOf;

	// responsible for calculating the next step asynchronously
	TFuture<void> AsyncState;


	void PostStateChange(int CellID);

	void ApplyCellRules();

	void TimestepPropertyShift();

	int GetCellAliveNeighbors(int CellID) const;

	// many arrays depend on the number of cells,
	// which is described by the Neighborhood array
	void PostNeighborhoodSetup();

	public:

	void InitializeCellStates(float Probability);
	void InitializeCellRules(FString BirthString, FString SurviveString);
	
	void SetBaseMembers(FBaseAutomataStruct NewBaseMembers) override;

	void StepComplete() override;
	void BroadcastData() override;
	void StartNewStep() override;
};

UCLASS()
class UAntRule : public UObject, public IAutomata
{
	GENERATED_BODY()

	FBaseAutomataStruct BaseMembers;

	// Ruleset that determines the tile's effect on the ant, e.g. {right, left} would be {1, -1}, or {1,3}
	TArray<int> CellSequence = { 1, -1 };

	// for each ant, describes its orientation: which neighbor it will move to to change its position
	TArray<int> AntOrientations = { 0 };

	// describes each ant's position, used to query cell states and neighborhoods
	TArray<int> AntPositions = { 0 };

	// responsible for calculating the next step asynchronously
	TFuture<void> AsyncState;

	void MoveAnts();

public:

	void SetBaseMembers(FBaseAutomataStruct NewBaseMembers) override;

	void InitializeAnts(int NumAnts);

	void StepComplete() override;
	void BroadcastData() override;
	void StartNewStep() override;

};

namespace AutomataFuncs {
	static void MakeNeighborsOf(TArray<TArray<int>>& NeighborsOf, TArray<TArray<int>>& Neighborhoods);

	static TArray<bool> StringToRule(FString RuleDigits);
}
