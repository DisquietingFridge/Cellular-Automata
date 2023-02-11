#include "Rulesets.h"

void ULifelikeRule::PostNeighborhoodSetup()
{
	int NumCells = BaseMembers.Neighborhoods.Num();

	BaseMembers.SwitchStepBuffer.Init(TNumericLimits<int32>::Min(), NumCells);

	NextStates.Init(false, NumCells);

	EvalFlaggedThisStep.Init(true, NumCells);
	EvalFlaggedLastStep.Init(true, NumCells);
}

void ULifelikeRule::SetNeighborhoods(TArray<TArray<int>> Neighbs)
{
	BaseMembers.Neighborhoods = Neighbs;

	AutomataFuncs::MakeNeighborsOf(NeighborsOf, BaseMembers.Neighborhoods);
	PostNeighborhoodSetup();
}

void ULifelikeRule::InitializeCellStates(float Probability)
{
	int NumCells = BaseMembers.Neighborhoods.Num();

	BaseMembers.CurrentStates.Reserve(NumCells);

	for (int i = 0; i < NumCells; ++i)
	{
		BaseMembers.CurrentStates.Add(FMath::FRandRange(0, TNumericLimits<int32>::Max() - 1) < Probability * TNumericLimits<int32>::Max());
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
	ParallelFor(BaseMembers.Neighborhoods.Num(), [&](int32 CellID)
	{
		if (EvalFlaggedLastStep[CellID])// register change based on state
		{
			// register change based on state
			if (NextStates[CellID])
			{  // switch-off time is in the future, i.e. cell is still on
				BaseMembers.SwitchStepBuffer[CellID] = TNumericLimits<float>::Max();
			}
			else // is off at next time
			{
				if (BaseMembers.CurrentStates[CellID])  // was previously on
				{ // register switch-off time as being upcoming step
					BaseMembers.SwitchStepBuffer[CellID] = BaseMembers.NextStep;
				}
			}
		}
	});
}

void ULifelikeRule::ApplyCellRules()
{
	ParallelFor(BaseMembers.Neighborhoods.Num(), [&](int32 CellID)
	{
		if (EvalFlaggedLastStep[CellID])
		{
			int AliveNeighbors = GetCellAliveNeighbors(CellID);

			NextStates[CellID] = BaseMembers.CurrentStates[CellID] ? int(SurviveRules[AliveNeighbors]) : int(BirthRules[AliveNeighbors]);

			//there has been a change of state
			if (NextStates[CellID] != BaseMembers.CurrentStates[CellID])
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
	++BaseMembers.NextStep;

	ParallelFor(BaseMembers.Neighborhoods.Num(), [&](int32 CellID)
	{
		BaseMembers.CurrentStates[CellID] = NextStates[CellID];
		EvalFlaggedLastStep[CellID] = EvalFlaggedThisStep[CellID];
		EvalFlaggedThisStep[CellID] = false;

	});
}

int ULifelikeRule::GetCellAliveNeighbors(int CellID) const
{
	//Query the cell's neighborhood to sum its alive neighbors
	int AliveNeighbors = 0;

	for (int Neighbor : BaseMembers.Neighborhoods[CellID])
	{
		AliveNeighbors += BaseMembers.CurrentStates[Neighbor];
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
	SwitchStepsReady.Broadcast(BaseMembers.SwitchStepBuffer);
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
		
		int& HostState = BaseMembers.CurrentStates[AntCell];

		// change ant orientation
		const TArray<int>& AntNeighborhood = BaseMembers.Neighborhoods[AntCell];
		int NumNeighbs = AntNeighborhood.Num();
		AntOrientation += CellSequence[HostState] + NumNeighbs;
		AntOrientation %= NumNeighbs;

		// change host cell state
		int OldHostState = HostState;
		HostState += 1;
		HostState %= NumStates;

		//update display data
		if (HostState != OldHostState)
		{
			BaseMembers.SwitchStepBuffer[AntCell] = 
				(HostState == 1) ? 
				TNumericLimits<float>::Max() : 
				BaseMembers.NextStep;
		}

		// move ant along
		AntCell = AntNeighborhood[AntOrientation];
	}
}

void UAntRule::PostNeighborhoodSetup()
{
	int NumCells = BaseMembers.Neighborhoods.Num();

	BaseMembers.SwitchStepBuffer.Init(TNumericLimits<int32>::Min(), NumCells);

	BaseMembers.CurrentStates.Init(0, NumCells);
}

void UAntRule::InitializeAnts(int NumAnts)
{
}

void UAntRule::SetNeighborhoods(TArray<TArray<int>> Neighbs)
{
	BaseMembers.Neighborhoods = Neighbs;
	PostNeighborhoodSetup();
}

void UAntRule::StepComplete()
{
	AsyncState.Wait();
	BaseMembers.NextStep++;
}

void UAntRule::BroadcastData()
{
	SwitchStepsReady.Broadcast(BaseMembers.SwitchStepBuffer);
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
