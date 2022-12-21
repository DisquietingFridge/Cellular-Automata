#pragma once

#include "Rulesets.generated.h"


UCLASS()
class ULifelikeRule : public UObject
{
	GENERATED_BODY()


	TArray<float> SwitchStepBuffer;
	float NextStep;

	//Set that stores the birth rules for the automata
	TArray<bool> BirthRules;
	//Set that stores the survival rules for the automata
	TArray<bool> SurviveRules;

	TArray<bool> CurrentStates;
	TArray<bool> NextStates;

	TArray<bool> EvalFlaggedThisStep;
	TArray<bool> EvalFlaggedLastStep;

	TArray<TArray<int>> Neighborhoods;
	TArray<TArray<int>> NeighborsOf;

	TFuture<void> AsyncState;

	void CellProcessorWork();

	

	void CalcStepSwitches();

	void ApplyCellRules();

	void TimestepPropertyShift();

	int GetCellAliveNeighbors(int CellID) const;


	public:

	DECLARE_EVENT_OneParam(ULifelikeRule, SendDisplayData, const TArray<float>&)

	SendDisplayData SwitchStepsReady;

	void InitializeCellStates(float Probability);
	void InitializeCellRules(FString BirthString, FString SurviveString);
	void StartingDataSetup();

	void SetNeighborhoods(TArray<TArray<int>> Neighbs)
	{
		Neighborhoods = Neighbs;
	}

	void SetNeighborsOf(TArray<TArray<int>> NeighbsOf)
	{
		NeighborsOf = NeighbsOf;
	}

	TSharedRef<TArray<float>> GetSwitchStepPtr()
	{
		TSharedPtr<TArray<float>> Temp = MakeShareable<TArray<float>>(&SwitchStepBuffer);
		return Temp.ToSharedRef();
	}

	void SetBroadcast(SendDisplayData Event);

	void StepComplete();
	void BroadcastData();
	void StartNewStep();


};