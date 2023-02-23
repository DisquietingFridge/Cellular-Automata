#include "Rulesets.h"
#include "AutomataDisplay.h"

void ULifelikeRule::PostNeighborhoodSetup()
{
	AutomataFuncs::MakeNeighborsOf(NeighborsOf, BaseMembers.Neighborhoods);

	int NumCells = BaseMembers.Neighborhoods.Num();

	NextStates.Init(false, NumCells);

	EvalFlaggedThisStep.Init(true, NumCells);
	EvalFlaggedLastStep.Init(true, NumCells);
}

void ULifelikeRule::Tick(float DeltaTime)
{
	BaseMembers.Display->UpdateSwitchTimes(SwitchStepOutput);
	Tickable = false;
}

void ULifelikeRule::InitializeCellStates(float Probability)
{
	for (int& State : BaseMembers.CurrentStates)
	{
		State = FMath::FRandRange(0, TNumericLimits<int32>::Max() - 1) < Probability* TNumericLimits<int32>::Max();
	}
}

void ULifelikeRule::InitializeCellRules(FString BirthString, FString SurviveString)
{
	BirthRules = AutomataFuncs::StringToRule(BirthString);
	SurviveRules = AutomataFuncs::StringToRule(SurviveString);
}

void ULifelikeRule::SetBaseMembers(FBaseAutomataStruct NewBaseMembers)
{
	BaseMembers = NewBaseMembers;
	PostNeighborhoodSetup();
}

void ULifelikeRule::ApplyCellRules()
{
	ParallelFor(BaseMembers.Neighborhoods.Num(), [&](int32 CellID)
	{
		if (EvalFlaggedLastStep[CellID])
		{
			int AliveNeighbors = GetCellAliveNeighbors(CellID);

			NextStates[CellID] =	BaseMembers.CurrentStates[CellID] ? 
									int(SurviveRules[AliveNeighbors]) : 
									int(BirthRules[AliveNeighbors]);

			PostStateChange(CellID);	
		}
	} /*,EParallelForFlags::ForceSingleThread*/);
}

void ULifelikeRule::PostStateChange(int CellID)
{
	if (NextStates[CellID] != BaseMembers.CurrentStates[CellID])
	{
		EvalFlaggedThisStep[CellID] = true;
		for (int InfluencedCellID : NeighborsOf[CellID])
		{
			EvalFlaggedThisStep[InfluencedCellID] = true;
		}

		BaseMembers.SwitchStepBuffer[CellID] =	NextStates[CellID] ? 
												TNumericLimits<float>::Max() : 
												BaseMembers.NextStep;
	}
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

void ULifelikeRule::StepComplete()
{
	AsyncState.Wait();

	TimestepPropertyShift();
}

void ULifelikeRule::BroadcastData()
{
	//BaseMembers.Display->UpdateSwitchTimes(BaseMembers.SwitchStepBuffer);
	SwitchStepOutput = BaseMembers.SwitchStepBuffer;
	Tickable = true;
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


		BaseMembers.SwitchStepBuffer[AntCell] = BaseMembers.NextStep;

		// move ant along
		AntCell = AntNeighborhood[AntOrientation];
	}
}

void UAntRule::SetBaseMembers(FBaseAutomataStruct NewBaseMembers)
{
	BaseMembers = NewBaseMembers;
}

void UAntRule::InitializeAnts(int NumAnts)
{
	AntPositions.Init(0, NumAnts);
	for (int& AntPos : AntPositions)
	{
		AntPos = FMath::RandRange(0, BaseMembers.Neighborhoods.Num() - 1);
	}

	AntOrientations.Init(0, NumAnts);
	for (int& AntOr : AntOrientations)
	{
		AntOr = FMath::RandRange(0, BaseMembers.Neighborhoods[0].Num() - 1);
	}
}

void UAntRule::InitializeSequence(TArray<int> Seq)
{
	CellSequence = Seq;
}

void UAntRule::StepComplete()
{
	AsyncState.Wait();
	BaseMembers.NextStep++;
}

void UAntRule::BroadcastData()
{
	BaseMembers.Display->UpdateSwitchTimes(BaseMembers.SwitchStepBuffer);
	BaseMembers.Display->UpdateEndFadeState(BaseMembers.CurrentStates);
}

void UAntRule::StartNewStep()
{
	AsyncState = Async(EAsyncExecution::TaskGraph, [&]() {MoveAnts(); });
	//MoveAnts();
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

TArray<bool> AutomataFuncs::StringToRule(FString RuleDigits)
{
	TArray<bool> Rule;
	Rule.Init(false, 10);

	for (TCHAR character : RuleDigits)
	{
		if (TChar<TCHAR>::IsDigit(character))
		{
			int32 index = TChar<TCHAR>::ConvertCharDigitToInt(character);
			Rule[index] = true;
		}
	}

	return Rule;
}
