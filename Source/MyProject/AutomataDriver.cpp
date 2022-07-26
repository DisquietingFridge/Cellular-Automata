#include "AutomataDriver.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/Char.h"
#include "Async/Async.h"
#include "Algo/Accumulate.h"
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

	CreateGridRuleInterface();

}

void AAutomataDriver::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitializeCellRules();

	InitializeCellStates();

	InitializeCellNeighborhoods();

	InitializeCellNeighborsOf();

	StartingDataSetup();
}


void AAutomataDriver::StartingDataSetup()
{
	NextStepTime = 0;

	SwitchTimeBuffer.Init(-2 * (StepPeriod * StepsToFade), NumCells());

	AsyncState = Async(EAsyncExecution::TaskGraph, Work);
}

// Called when the game starts or when spawned
void AAutomataDriver::BeginPlay()
{
	Super::BeginPlay();

	InitializeNiagaraSystem();
	StepComplete();
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
			GridCoords[ID] = { x, z };
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

	NiagaraComponent->ActivateSystem();
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
}

void AAutomataDriver::InitializeCellNeighborhoods()
{
	Neighborhoods.Init(nullptr, NumCells());

	ParallelFor(NumCells(), [&](int CellID)
	{
		TArray<TPair<FIntPoint, FIntPoint>> NeighborInfo;
		NeighborInfo.Reserve(RelativeNeighborhood.Num());

		TArray<FIntPoint> NeighborCoords = RelativeNeighborhood;
		FIntPoint CellCoords = GridCoords[CellID];

		switch (Shape)
		{
		case (CellShape::Square):
			for (int i = 0; i < RelativeNeighborhood.Num(); ++i)
			{
				NeighborCoords[i] += CellCoords;
				NeighborInfo.Add(TPair<FIntPoint, FIntPoint>(RelativeNeighborhood[i], NeighborCoords[i]));
			}
			break;

		case (CellShape::Hex):
			FIntPoint AxialCoord = HexCoords::OffsetToAxial(CellCoords);
			for (int i = 0; i < RelativeNeighborhood.Num(); ++i)
			{
				NeighborCoords[i] += AxialCoord;
				FIntPoint OffsetPoint = HexCoords::AxialToOffset(NeighborCoords[i]);
				NeighborCoords[i] = OffsetPoint;

				NeighborInfo.Add(TPair<FIntPoint, FIntPoint>(RelativeNeighborhood[i], NeighborCoords[i]));
			}
			break;
		}
		Neighborhoods[CellID] = GridRule->MapNeighborhood(NeighborInfo);

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
		for (TPair<FIntPoint,int> Neighbor : *(Neighborhoods[i]))
		{
			int NeighborID = Neighbor.Value;
			NeighborsOf[NeighborID]->Add(i);
		}
	}
}


void AAutomataDriver::SetCellNextCustomData()
{
	ParallelFor(NumCells(), [&](int32 CellID)
	{
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
	});
}


void AAutomataDriver::ApplyCellRules()
{
	ParallelFor(NumCells(), [&](int32 CellID)
	{

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

int AAutomataDriver::GetCellAliveNeighbors(const int CellID) const
{
	//Query the cell's neighborhood to sum its alive neighbors
	int AliveNeighbors = 0;
	TMap<FIntPoint,int> Neighborhood = *(Neighborhoods[CellID]);
	for (TPair<FIntPoint,int> Neighbor: Neighborhood)
	{
		AliveNeighbors += CurrentStates[Neighbor.Value];
	}
	return AliveNeighbors;
}


void AAutomataDriver::CellProcessorWork()
{
	ApplyCellRules();

	SetCellNextCustomData();
}

void AAutomataDriver::StepComplete()
{
	AsyncState.Wait();

	TimestepPropertyShift();

	// kick off calculation of next stage
	AsyncState = Async(EAsyncExecution::TaskGraph, Work);
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

	});
}

void AAutomataDriver::TimerFired()
{
	StepComplete();
}

UStandardXZGrid::UStandardXZGrid()
{
	IGridSpecsInterface* Grid = Cast<IGridSpecsInterface>(GetOuter());
	if (Grid)
	{
		Tie(NumXCells, NumZCells) = Grid->GetGridDimensions();
	}
}

int UStandardXZGrid::ApplyRule(FIntPoint Coord) const
{
	return -1;
}

TSharedPtr<TArray<int>> UStandardXZGrid::RawCoordsToCellIDs(TArray<FIntPoint>& RawCoords) const
{
	TArray<int> CellIDs;
	for (FIntPoint Coord : RawCoords)
	{
		int CellID = ApplyRule(Coord);
		if (CellID >= 0)
		{
			CellIDs.Add(CellID);
		}
	}
	TSharedPtr<TArray<int>> Pointer = MakeShared<TArray<int>>(CellIDs);
	return Pointer;
}

TSharedPtr<TMap<FIntPoint, int>> UStandardXZGrid::MapNeighborhood(TArray<TPair<FIntPoint, FIntPoint>> NeighborInfo) const
{
	TMap<FIntPoint,int> NeighborhoodMap;

	for (int i = 0 ; i < NeighborInfo.Num(); ++i)
	{
		FIntPoint RawCoord = NeighborInfo[i].Value;
		int CellID = ApplyRule(RawCoord);
		if (CellID >= 0)
		{
			FIntPoint Key = NeighborInfo[i].Key;
			NeighborhoodMap.Add(Key, CellID);
		}
	}
	TSharedPtr<TMap<FIntPoint, int>> Pointer = MakeShared<TMap<FIntPoint, int>>(NeighborhoodMap);
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

int USphereRule::ApplyRule(FIntPoint Coord) const
{
	return -1;
}

int UCrossSurfaceRule::ApplyRule(FIntPoint Coord) const
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

	return CoordToCellID(Coord);
}

int UKleinRule::ApplyRule(FIntPoint Coord) const
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

	return CoordToCellID(Coord);

}

int UTorusRule::ApplyRule(FIntPoint Coord) const
{
	LoopAxis(Coord, DeformedAxis::XAxis);
	LoopAxis(Coord, DeformedAxis::ZAxis);

	return CoordToCellID(Coord);
}

int UCylinderRule::ApplyRule(FIntPoint Coord) const
{
	if ((Coord[1] >= 0) && (Coord[1] < NumZCells))
	{
		LoopAxis(Coord, DeformedAxis::XAxis);

		return CoordToCellID(Coord);
	}
	else return -1;
}

int UFiniteRule::ApplyRule(FIntPoint Coord) const
{
	int x = Coord[0];
	int z = Coord[1];

	if (((x >= 0) && (z >= 0)) && ((x < NumXCells) && (z < NumZCells)))
	{
		return CoordToCellID(Coord);
	}
	else return -1;
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