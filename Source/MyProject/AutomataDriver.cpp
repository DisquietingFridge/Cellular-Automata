#include "AutomataDriver.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/Char.h"
#include "Async/Async.h"
#include "Async/AsyncWork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "HexCoords.h"

typedef UNiagaraDataInterfaceArrayFunctionLibrary NiagaraFuncs;


// Sets default values
AAutomataDriver::AAutomataDriver()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Root Component"));
}


void AAutomataDriver::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	SetGridCoords();

	SetRelativeNeighborhood();

	InitializeCellTransforms();

	InitializeMaterial();

	InitializeNiagaraSystem();

	CreateGridRuleInterface();

}

void AAutomataDriver::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitializeCellRules();

	InitializeCellStates();

	InitializeCellNeighborhoods();

	GridCoords = TArray<FIntPoint>();

	InitializeCellNeighborsOf();

	InitializeCellProcessors();

	StartingDataSetup();
}


void AAutomataDriver::StartingDataSetup()
{
	NextStepTime = 0;

	SwitchTimeBuffer.Init(-2 * (StepPeriod * StepsToFade), NumCells());

	Processors[0]->StartSynchronousTask();

}

// Called when the game starts or when spawned
void AAutomataDriver::BeginPlay()
{
	Super::BeginPlay();

	StepComplete();
	NiagaraComponent->ActivateSystem();

	// we are ready to start the iteration steps.
	GetWorldTimerManager().SetTimer(StepTimer, this, &AAutomataDriver::TimerFired, StepPeriod, true);
}

void AAutomataDriver::SetGridCoords()
{
	GridCoords.Init(FIntPoint(), NumCells());
	ParallelFor(NumZCells, [&](int z)
	{
		ParallelFor(NumXCells, [&](int x)
		{
			int ID = z * NumXCells + x;
			GridCoords[ID] = FIntPoint(x, z);
		});
	});
}

void AAutomataDriver::SetRelativeNeighborhood()
{
	switch (Shape)
	{
	case CellShape::Square:
		RelativeNeighborhood = RelativeMooreNeighborhood;
		break;
	case CellShape::Hex:
		RelativeNeighborhood = RelativeAxialNeighborhood;
		break;
	default:
		RelativeNeighborhood = RelativeMooreNeighborhood;
	}
}

void AAutomataDriver::InitializeMaterial()
{
	// Create material and set up properties
	DynMaterial = UMaterialInstanceDynamic::Create(Mat, this);

	DynMaterial->SetScalarParameterValue("PhaseExponent", PhaseExponent);
	DynMaterial->SetScalarParameterValue("EmissiveMultiplier", EmissiveMultiplier);
	DynMaterial->SetVectorParameterValue("OnColor", OnColor);
	DynMaterial->SetVectorParameterValue("OffColor", OffColor);
	DynMaterial->SetScalarParameterValue("FadePerSecond", 1 / (StepPeriod * StepsToFade));

	DynMaterial->SetScalarParameterValue("IsHexagon", Shape == CellShape::Hex);
}

void AAutomataDriver::InitializeCellTransforms()
{
	CellTransforms.Reserve(NumCells());

	for (int i = 0; i < NumCells(); ++i)
	{
		FIntPoint Coord = GridCoords[i];
		FVector TransformResult;
		switch (Shape)
		{
		case CellShape::Square:
			TransformResult = Offset * FVector(Coord[0], 0, Coord[1]);
			break;
		case CellShape::Hex:
			FVector2D TwoDeeTransform = HexCoords::OffsetToTransform(Coord);
			TransformResult = Offset * FVector(TwoDeeTransform[0], 0, TwoDeeTransform[1]);
		}
		CellTransforms.Add(TransformResult);
	}
}

void AAutomataDriver::InitializeNiagaraSystem()
{
	NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(ParticleSystem, RootComponent, FName(), FVector(0), FRotator(0), EAttachLocation::KeepRelativeOffset, false, false, ENCPoolMethod::None, true);

	NiagaraFuncs::SetNiagaraArrayVector(NiagaraComponent, "User.Transforms", CellTransforms);
	NiagaraComponent->SetVariableMaterial(FName("User.Material"), DynMaterial);
	NiagaraComponent->SetVariableInt(FName("User.XCount"), NumXCells);
	NiagaraComponent->SetVariableInt(FName("User.ZCount"), NumZCells);
}

void AAutomataDriver::InitializeCellRules()
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

void AAutomataDriver::InitializeCellStates()
{
	CurrentStates.Reserve(NumCells());

	for (int i = 0; i < NumCells(); ++i)
	{
		CurrentStates.Add(FMath::FRandRange(0, TNumericLimits<int32>::Max() - 1) < Probability * TNumericLimits<int32>::Max());
	}

	NextStates.Init(false, NumCells());

	EvalFlaggedThisStep.Init(true, NumCells());
	EvalFlaggedLastStep.Init(true, NumCells());

}


void AAutomataDriver::CreateGridRuleInterface()
{
	GridRuleFactory = NewObject<UBaseGridRuleFactory>(this, GridRuleFactoryType);
	GridRule = GridRuleFactory->CreateGridRuleInterface(SelectedGridRule);
	GridRule->InitializeGridParams(NumXCells, NumZCells);
}

void AAutomataDriver::InitializeCellNeighborhoods()
{
	Neighborhoods.Init(nullptr, NumCells());

	ParallelFor(NumCells(), [&](int CellID)
	{
		TArray<FIntPoint> NeighborCoords = RelativeNeighborhood;
		FIntPoint CellCoords = GridCoords[CellID];

		switch (Shape)
		{
		case (CellShape::Square):
			for (int i = 0; i < RelativeNeighborhood.Num(); ++i)
			{
				NeighborCoords[i] += CellCoords;
			}
			break;

		case (CellShape::Hex):
			FIntPoint AxialCoord = HexCoords::OffsetToAxial(CellCoords);
			for (int i = 0; i < RelativeNeighborhood.Num(); ++i)
			{
				NeighborCoords[i] += AxialCoord;
				FIntPoint OffsetPoint = HexCoords::AxialToOffset(NeighborCoords[i]);
				NeighborCoords[i] = OffsetPoint;
			}
			break;
		}
		Neighborhoods[CellID] = GridRule->RawCoordsToCellIDs(NeighborCoords);
	});
}

void AAutomataDriver::InitializeCellNeighborsOf()
{
	NeighborsOf.Init(nullptr, NumCells());
	ParallelFor(NumCells(), [&](int i)
	{
		NeighborsOf[i] = MakeShared<TSet<int>>(TSet<int>());
	});

	for (int i = 0; i < NumCells(); ++i)
	{
		for (int Neighbor : *(Neighborhoods[i]))
		{
			NeighborsOf[Neighbor]->Add(i);
		}
	}
}

void AAutomataDriver::InitializeCellProcessors()
{

	TArray<int> ProcessorCells;
	ProcessorCells.Reserve(NumCells());
	for (int i = 0; i < NumCells(); ++i)
	{
		ProcessorCells.Add(i);
	}
	FAsyncTask<CellProcessor>* NewProcessor = new FAsyncTask<CellProcessor>(this, ProcessorCells);
	Processors.Add(NewProcessor);
}


void AAutomataDriver::SetCellNextCustomData(const TArray<int>& CellIDs)
{
	ParallelFor(CellIDs.Num(), [&](int32 i)
	{
		int CellID = CellIDs[i];

		if (EvalFlaggedLastStep[CellID])// register change based on state
		{
			// register change based on state
			if (NextStates[CellID])
			{  // switch-off time is in the future, i.e. cell is still on
				SwitchTimeBuffer[CellID] = TNumericLimits<float>::Max();
			}
			else // is off at next time
			{
				if (CurrentStates[CellID])  // was previously on
				{ // register switch-off time as being upcoming step
					SwitchTimeBuffer[CellID] = NextStepTime;
				}
			}
		}
	}/*, EParallelForFlags::BackgroundPriority*/);
}

void AAutomataDriver::SetCellNextCustomData(const int CellID)
{
	TArray<int> PackagedCellID{ CellID };
	SetCellNextCustomData(PackagedCellID);
}


void AAutomataDriver::ApplyCellRules(const TArray<int>& CellIDs)
{
	ParallelFor(CellIDs.Num(), [&](int32 i)
	{
		const int CellID = CellIDs[i];

		if (EvalFlaggedLastStep[CellID])
		{
			int AliveNeighbors = GetCellAliveNeighbors(CellID);

			NextStates[CellID] = CurrentStates[CellID] ? SurviveRules[AliveNeighbors] : BirthRules[AliveNeighbors];

			//there has been a change of state
			if (NextStates[CellID] != CurrentStates[CellID])
			{
				EvalFlaggedThisStep[CellID] = true;
				for (int InfluencedCellID : *(NeighborsOf[CellID]))
				{
					EvalFlaggedThisStep[InfluencedCellID] = true;
				}
			}
		}
	});
}

void AAutomataDriver::ApplyCellRules(const int CellID)
{
	TArray<int> PackagedCellID{ CellID };
	ApplyCellRules(PackagedCellID);
}

int AAutomataDriver::GetCellAliveNeighbors(const int CellID) const
{
	//Query the cell's neighborhood to sum its alive neighbors
	int AliveNeighbors = 0;
	TArray<int> Neighborhood = *(Neighborhoods[CellID]);
	for (int NeighborID : Neighborhood)
	{
		AliveNeighbors += CurrentStates[NeighborID];
	}
	return AliveNeighbors;
}

void AAutomataDriver::CellProcessorWork(const TArray<int>& CellIDs)
{
	ApplyCellRules(CellIDs);

	SetCellNextCustomData(CellIDs);
}

void AAutomataDriver::StepComplete()
{

	// have all the cells' next state calculated before sending to material
	// strictly speaking we only need to check the last one, but
	// checking all for safety
	for (FAsyncTask<CellProcessor>* Process : Processors)
	{
		Process->EnsureCompletion(false);
	}

	TimestepPropertyShift();

	// kick off calculation of next stage
	Processors[0]->StartBackgroundTask();
}

void AAutomataDriver::TimestepPropertyShift()
{
	NextStepTime = GetWorld()->GetTimeSeconds() + StepPeriod;

	NiagaraFuncs::SetNiagaraArrayFloat(NiagaraComponent, "User.SwitchTimes", SwitchTimeBuffer);

	ParallelFor(NumCells(), [&](int32 CellID)
	{
		CurrentStates[CellID] = NextStates[CellID];

		EvalFlaggedLastStep[CellID] = EvalFlaggedThisStep[CellID];
		EvalFlaggedThisStep[CellID] = false;

	}/*, EParallelForFlags::BackgroundPriority*/);
}

void AAutomataDriver::TimerFired()
{
	StepComplete();
}

CellProcessor::CellProcessor(AAutomataDriver* Driver, TArray<int> CellIDs)
{
	this->Driver = Driver;
	this->CellIDs = CellIDs;
}

// Calculate state transitions for the cells this processor is responsible for
void CellProcessor::DoWork()
{
	Driver->CellProcessorWork(CellIDs);
}

void UStandardXZGrid::InitializeGridParams(int NumXCellsInput, int NumZCellsInput)
{
	NumXCells = NumXCellsInput;
	NumZCells = NumZCellsInput;
}

void UStandardXZGrid::ApplyRule(TArray<int>& CellIDs, FIntPoint Coord) const
{
}

TSharedPtr<TArray<int>> UStandardXZGrid::RawCoordsToCellIDs(TArray<FIntPoint>& RawCoords) const
{
	TArray<int> CellIDs;
	for (FIntPoint Coord : RawCoords)
	{
		ApplyRule(CellIDs, Coord);
	}
	TSharedPtr<TArray<int>> Pointer = MakeShared<TArray<int>>(CellIDs);
	return Pointer;
}

bool UStandardXZGrid::IsAxisTwisted(FIntPoint Coord, DeformedAxis TwistedAxis) const
{
	int NumAxisCells = 0;
	int Index = 0;

	switch (TwistedAxis)
	{
	case DeformedAxis::XAxis:
		NumAxisCells = NumXCells;
		Index = 0;
		break;
	case DeformedAxis::ZAxis:
		NumAxisCells = NumZCells;
		Index = 1;
		break;
	}

	if (Coord[Index] < 0)
	{
		return !(bool)(((abs(Coord[Index]) - 1) / NumAxisCells) % 2);
	}
	else
	{
		return bool((Coord[Index] / NumAxisCells) % 2);
	}
}

void UStandardXZGrid::ParamsFromAxis(int& Index, int& NumAxisCells, DeformedAxis Axis) const
{
	switch (Axis)
	{
	case DeformedAxis::XAxis:
		NumAxisCells = NumXCells;
		Index = 0;
		break;
	case DeformedAxis::ZAxis:
		NumAxisCells = NumZCells;
		Index = 1;
		break;
	}
}

void UStandardXZGrid::LoopAxis(FIntPoint& Coord, DeformedAxis AxisToLoop) const
{
	int NumAxisCells;
	int Index;
	ParamsFromAxis(Index, NumAxisCells, AxisToLoop);

	if (Coord[Index] >= 0)
	{
		Coord[Index] = Coord[Index] % NumAxisCells;
	}
	else
	{
		Coord[Index] = NumAxisCells - (abs(Coord[Index]) % NumAxisCells);
	}
}

void UStandardXZGrid::ReverseAxis(FIntPoint& Coord, DeformedAxis AxisToReverse) const
{
	int NumAxisCells = 0;
	int Index = 0;
	ParamsFromAxis(Index, NumAxisCells, AxisToReverse);

	LoopAxis(Coord, AxisToReverse);
	Coord[Index] = NumAxisCells - Coord[Index] - 1;
}

void USphereRule::ApplyRule(TArray<int>& CellIDs, FIntPoint Coord) const
{
}

void UCrossSurfaceRule::ApplyRule(TArray<int>& CellIDs, FIntPoint Coord) const
{

	bool ZAxisTwisted = IsAxisTwisted(Coord, DeformedAxis::ZAxis);
	bool XAxisTwisted = IsAxisTwisted(Coord, DeformedAxis::XAxis);

	if (XAxisTwisted)
	{
		ReverseAxis(Coord, DeformedAxis::ZAxis);
	}
	else
	{
		LoopAxis(Coord, DeformedAxis::ZAxis);
	}

	if (ZAxisTwisted)
	{
		ReverseAxis(Coord, DeformedAxis::XAxis);
	}
	else
	{
		LoopAxis(Coord, DeformedAxis::XAxis);
	}

	CellIDs.Add(CoordToCellID(Coord));
}

void UKleinRule::ApplyRule(TArray<int>& CellIDs, FIntPoint Coord) const
{

	bool ZAxisTwisted = IsAxisTwisted(Coord, DeformedAxis::ZAxis);

	if (ZAxisTwisted)
	{
		ReverseAxis(Coord, DeformedAxis::XAxis);
	}
	else
	{
		LoopAxis(Coord, DeformedAxis::XAxis);
	}

	LoopAxis(Coord, DeformedAxis::ZAxis);

	CellIDs.Add(CoordToCellID(Coord));

}

void UTorusRule::ApplyRule(TArray<int>& CellIDs, FIntPoint Coord) const
{
	LoopAxis(Coord, DeformedAxis::XAxis);
	LoopAxis(Coord, DeformedAxis::ZAxis);

	CellIDs.Add(CoordToCellID(Coord));
}

void UCylinderRule::ApplyRule(TArray<int>& CellIDs, FIntPoint Coord) const
{
	if ((Coord[1] >= 0) && (Coord[1] < NumZCells))
	{
		LoopAxis(Coord, DeformedAxis::XAxis);

		CellIDs.Add(CoordToCellID(Coord));
	}
}

void UFiniteRule::ApplyRule(TArray<int>& CellIDs, FIntPoint Coord) const
{
	int x = Coord[0];
	int z = Coord[1];

	if (((x >= 0) && (z >= 0)) && ((x < NumXCells) && (z < NumZCells)))
	{
		CellIDs.Add(CoordToCellID(Coord));
	}
}

IGridRuleInterface* UBaseGridRuleFactory::CreateGridRuleInterface(BoundGridRuleset Ruleset)
{
	const TSubclassOf<UStandardXZGrid>* RuleTypeResult = RuleEnumToClass.Find(Ruleset);
	TSubclassOf<class UStandardXZGrid> RuleType = RuleTypeResult ? *RuleTypeResult : UStandardXZGrid::StaticClass();

	UObject* CreatedObject = NewObject<UStandardXZGrid>(this->GetOuter(), RuleType);

	IGridRuleInterface* GridRule;
	GridRule = Cast<IGridRuleInterface>(CreatedObject);
	return GridRule;
}