#include "Rulesets.h"

void ULifelikeRule::PostNeighborhoodSetup()
{
	int NumCells = Neighborhoods.Num();

	//TODO: should this start at 1?
	NextStep = 0;

	SwitchStepBuffer.Init(-2, NumCells);

	NextStates.Init(false, NumCells);

	EvalFlaggedThisStep.Init(true, NumCells);
	EvalFlaggedLastStep.Init(true, NumCells);
}

void ULifelikeRule::SetNeighborhoods(TArray<TArray<int>> Neighbs)
{
	Neighborhoods = Neighbs;

	AutomataFuncs::MakeNeighborsOf(NeighborsOf, Neighborhoods);
	PostNeighborhoodSetup();
}

void ULifelikeRule::InitializeCellStates(float Probability)
{
	int NumCells = Neighborhoods.Num();

	CurrentStates.Reserve(NumCells);

	for (int i = 0; i < NumCells; ++i)
	{
		CurrentStates.Add(FMath::FRandRange(0, TNumericLimits<int32>::Max() - 1) < Probability * TNumericLimits<int32>::Max());
	}
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
	AsyncState = Async(EAsyncExecution::TaskGraph, [&]() {ApplyCellRules(); });

}

void UAntRule::MoveAnts()
{
	int NumAnts = AntPositions.Num();
	int NumStates = CellSequence.Num();

	for (int Ant = 0; Ant < NumAnts; ++Ant)
	{

		int& AntCell = AntPositions[Ant];
		int& AntOrientation = AntOrientations[Ant];

		TArray<int> AntNeighborhood = Neighborhoods[AntCell];
		int NumNeighbs = AntNeighborhood.Num();
		
		int& HostState = CurrentStates[AntCell];
		int OldHostState = HostState;

		// change ant orientation
		AntOrientation += CellSequence[HostState] + NumNeighbs;
		AntOrientation %= NumNeighbs;

		// change host cell state
		HostState += 1;
		HostState %= NumStates;

		//update display data
		if (HostState != OldHostState)
		{
			if (HostState == 1)
			{
				SwitchStepBuffer[AntCell] = TNumericLimits<float>::Max();
			}
			else
			{
				SwitchStepBuffer[AntCell] = NextStep;
			}
		}

		// move ant along
		AntCell = AntNeighborhood[AntOrientation];
	}
}

void UAntRule::PostNeighborhoodSetup()
{
	int NumCells = Neighborhoods.Num();

	NextStep = 0;

	SwitchStepBuffer.Init(-2, NumCells);

	CurrentStates.Init(0, NumCells);
}

void UAntRule::InitializeAnts(int NumAnts)
{
}

void UAntRule::SetNeighborhoods(TArray<TArray<int>> Neighbs)
{
	Neighborhoods = Neighbs;
	PostNeighborhoodSetup();
}

void UAntRule::StepComplete()
{
	AsyncState.Wait();
	NextStep++;
}

void UAntRule::BroadcastData()
{
	SwitchStepsReady.Broadcast(SwitchStepBuffer);
}

void UAntRule::StartNewStep()
{
	AsyncState = Async(EAsyncExecution::TaskGraph, [&]() {MoveAnts(); });
	//MoveAnts();
}

void UAntRule::SetBroadcast(SendDisplayData Event)
{
	SwitchStepsReady = Event;
}


static void AutomataFuncs::MakeNeighborsOf(TArray<TArray<int>>& NeighborsOf, TArray<TArray<int>>& Neighborhoods)
{
	TArray<int> MemoryDummy;
	NeighborsOf.Init(MemoryDummy, Neighborhoods.Num());

	TSet<int> SetDummy;
	TArray<TSet<int>> DupeGuard;
	DupeGuard.Init(SetDummy, NeighborsOf.Num());

	for (int i = 0; i < NeighborsOf.Num(); ++i)
	{
		for (int Neighbor : Neighborhoods[i])
		{
			DupeGuard[Neighbor].Add(i);
		}
	}

	for (int i = 0; i < NeighborsOf.Num(); ++i)
	{
		NeighborsOf[i] = DupeGuard[i].Array();
	}
}
