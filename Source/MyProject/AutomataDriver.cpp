#include "AutomataDriver.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/Char.h"
#include "Async/Async.h"

#include "NiagaraComponent.h"
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

	// TODO: move gridrule construction (and destruction) into existing Neighborhoods creation function
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

	AsyncState = Async(EAsyncExecution::TaskGraph, [&]() {CellProcessorWork();});
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
	using namespace HexCoords;
	CellTransforms.Reserve(NumCells());

	auto VectorConversion = [&](FVector2D Point)
	{
		return Offset * FVector(Point[0], 0, Point[1]);
	};

	for (int i = 0; i < NumCells(); ++i)
	{
		FIntPoint Coord = GridCoords[i];
		FVector TransformResult;
		switch (Shape)
		{
		case CellShape::Square:
			TransformResult = VectorConversion(Coord);
			break;
		case CellShape::Hex:
			TransformResult = VectorConversion(OffsetToTransform(Coord));
			break;
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
	// TODO: remove GridRuleFactory as member variable?
	GridRuleFactory = NewObject<UBaseGridRuleFactory>(this, GridRuleFactoryType);
	GridRule = GridRuleFactory->CreateGridRuleInterface(SelectedGridRule);
}

void AAutomataDriver::InitializeCellNeighborhoods()
{
	using namespace HexCoords;

	Neighborhoods.Init(nullptr, NumCells());

	ParallelFor(NumCells(), [&](int CellID)
	{
		TArray<TPair<FIntPoint, FIntPoint>> NeighborInfo;
		NeighborInfo.Reserve(RelativeNeighborhood.Num());

		TArray<FIntPoint> NeighborCoords = RelativeNeighborhood;
		FIntPoint CellCoord = GridCoords[CellID];

		switch (Shape)
		{
		case (CellShape::Square):
			for (int i = 0; i < RelativeNeighborhood.Num(); ++i)
			{
				NeighborCoords[i] += CellCoord;
				NeighborInfo.Add(TPair<FIntPoint, FIntPoint>(RelativeNeighborhood[i], NeighborCoords[i]));
			}
			break;

		case (CellShape::Hex):
			FIntPoint AxialCoord = OffsetToAxial(CellCoord);
			for (int i = 0; i < RelativeNeighborhood.Num(); ++i)
			{
				NeighborCoords[i] += AxialCoord;
				NeighborCoords[i] = AxialToOffset(NeighborCoords[i]);
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
	AsyncState = Async(EAsyncExecution::TaskGraph, [&]() {CellProcessorWork();});
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

