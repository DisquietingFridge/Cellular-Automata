#include "Rulesets.h"

void ULifelikeRule::StartingDataSetup()
{
	//TODO: should this start at 1?
	NextStep = 0;

	SwitchStepBuffer.Init(-2, Neighborhoods.Num());
}

void ULifelikeRule::CellProcessorWork()
{
	ApplyCellRules();

	//CalcStepSwitches();
}

void ULifelikeRule::InitializeCellStates(float Probability)
{
	int NumCells = Neighborhoods.Num();

	CurrentStates.Reserve(NumCells);

	for (int i = 0; i < NumCells; ++i)
	{
		CurrentStates.Add(FMath::FRandRange(0, TNumericLimits<int32>::Max() - 1) < Probability * TNumericLimits<int32>::Max());
	}

	NextStates.Init(false, NumCells);

	EvalFlaggedThisStep.Init(true, NumCells);
	EvalFlaggedLastStep.Init(true, NumCells);

}

void ULifelikeRule::InitializeCellRules(FString BirthString, FString SurviveString)
{
	BirthRules.Init(false, 10);
	SurviveRules.Init(false, 10);

	for (TCHAR character : BirthString)
	{
		if (TChar<TCHAR>::IsDigit(character))
		{
			int32 index = TChar<TCHAR>::ConvertCharDigitToInt(character);
			BirthRules[index] = true;
		}
	}

	for (TCHAR character : SurviveString)
	{
		if (TChar<TCHAR>::IsDigit(character))
		{
			int32 index = TChar<TCHAR>::ConvertCharDigitToInt(character);
			SurviveRules[index] = true;
		}
	}
}

void ULifelikeRule::CalcStepSwitches()
{
	ParallelFor(Neighborhoods.Num(), [&](int32 CellID)
	{
		if (EvalFlaggedLastStep[CellID])// register change based on state
		{
			// register change based on state
			if (NextStates[CellID])
			{  // switch-off time is in the future, i.e. cell is still on
				SwitchStepBuffer[CellID] = TNumericLimits<float>::Max();
			}
			else // is off at next time
			{
				if (CurrentStates[CellID])  // was previously on
				{ // register switch-off time as being upcoming step
					SwitchStepBuffer[CellID] = NextStep;
				}
			}
		}
	});
}

void ULifelikeRule::ApplyCellRules()
{
	ParallelFor(Neighborhoods.Num(), [&](int32 CellID)
	{
		if (EvalFlaggedLastStep[CellID])
		{
			int AliveNeighbors = GetCellAliveNeighbors(CellID);

			NextStates[CellID] = CurrentStates[CellID] ? SurviveRules[AliveNeighbors] : BirthRules[AliveNeighbors];

			//there has been a change of state
			if (NextStates[CellID] != CurrentStates[CellID])
			{
				EvalFlaggedThisStep[CellID] = true;
				for (int InfluencedCellID : NeighborsOf[CellID])
				{
					EvalFlaggedThisStep[InfluencedCellID] = true;
				}
			}
		}
	} /*,EParallelForFlags::ForceSingleThread*/);
}

void ULifelikeRule::TimestepPropertyShift()
{
	++NextStep;

	ParallelFor(Neighborhoods.Num(), [&](int32 CellID)
	{
		CurrentStates[CellID] = NextStates[CellID];
		EvalFlaggedLastStep[CellID] = EvalFlaggedThisStep[CellID];
		EvalFlaggedThisStep[CellID] = false;

	});
}

int ULifelikeRule::GetCellAliveNeighbors(int CellID) const
{
	//Query the cell's neighborhood to sum its alive neighbors
	int AliveNeighbors = 0;

	for (int Neighbor : Neighborhoods[CellID])
	{
		AliveNeighbors += CurrentStates[Neighbor];
	}
	return AliveNeighbors;
}

void ULifelikeRule::SetBroadcast(SendDisplayData Event)
{
	SwitchStepsReady = Event;
}

void ULifelikeRule::StepComplete()
{
	AsyncState.Wait();

	CalcStepSwitches();

	TimestepPropertyShift();
}

void ULifelikeRule::BroadcastData()
{
	SwitchStepsReady.Broadcast(SwitchStepBuffer);
}

void ULifelikeRule::StartNewStep()
{
	// kick off calculation of next stage
	AsyncState = Async(EAsyncExecution::TaskGraph, [&]() {CellProcessorWork();});

}


