#include "AutomataFactory.h"
#include "GridRules.h"
#include "Rulesets.h"
#include "AutomataDisplay.h"
#include "AutomataStepDriver.h"
#include "AutomataInterface.h"

// Sets default values
AAutomataFactory::AAutomataFactory()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Root Component"));
}

// Called when the game starts or when spawned
void AAutomataFactory::BeginPlay()
{
	Super::BeginPlay();

	AutomataInterfacePtr->BroadcastData();
	AutomataInterfacePtr->StartNewStep();
	Driver->SetTimer(StepPeriod);
}

void AAutomataFactory::PreInitializeComponents()
{
	Super::PreInitializeComponents();

}

void AAutomataFactory::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GridSetup();
	RuleCalcSetup();
	DisplaySetup();
	DriverSetup();
}

void AAutomataFactory::GridSetup()
{
	Grid = NewObject<UGridSpecs>(this);
	Grid->SetAndInit(MakeTuple(NumXCells, NumZCells), Offset, Shape);

	SetRelativeNeighborhood();

	UNeighborhoodMaker* Maker = NewObject<UNeighborhoodMaker>(this);
	Maker->Initialize(Grid);
	Maker->MakeNeighborhoods(Neighborhoods, RelativeNeighborhood, SelectedGridRule);
}

void AAutomataFactory::SetRelativeNeighborhood()
{
	if (AutomataType == UAntRule::StaticClass())
	{
		RelativeNeighborhood = RelativeCardinalNeighborhood;
		return;
	}

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
		break;
	}
}

void AAutomataFactory::RuleCalcSetup()
{
	Automata = NewObject<UObject>(GetWorld(), AutomataType);

	AutomataInterfacePtr = Cast<IAutomata>(Automata);
	if (AutomataInterfacePtr != nullptr)
	{
		AutomataInterfacePtr->SetNeighborhoods(Neighborhoods);
	}
	

	ULifelikeRule* Lifelike = Cast<ULifelikeRule>(Automata);
	if (Lifelike != nullptr)
	{
		Lifelike->InitializeCellRules(BirthString, SurviveString);
		Lifelike->InitializeCellStates(Probability);
		return;
	}

}

void AAutomataFactory::DisplaySetup()
{
	if (ParticleSystem == nullptr)
	{
		return;
	}

	Display = NewObject<UAutomataDisplay>(GetWorld());

	TMap<FName, FLinearColor> VecMap;
	VecMap.Add("OnColor",OnColor);
	VecMap.Add("OffColor", OffColor);

	TMap<FName, float> ScalarMap;
	ScalarMap.Add("StepPeriod", StepPeriod);
	ScalarMap.Add("PhaseExponent",PhaseExponent);
	ScalarMap.Add("EmissiveMultiplier",EmissiveMultiplier);
	ScalarMap.Add("FadePerSecond", 1 / (StepPeriod * StepsToFade));
	ScalarMap.Add("IsHexagon" , float(Shape == CellShape::Hex));

	Display->InitMaterial(Mat, ScalarMap, VecMap);

	Display->InitializeNiagaraSystem(ParticleSystem, RootComponent, Grid);

	IAutomata::SendDisplayData DisplayLink;
	DisplayLink.AddUObject(Display, &UAutomataDisplay::UpdateDisplay);

	if (AutomataInterfacePtr != nullptr)
	{
		AutomataInterfacePtr->SetBroadcast(DisplayLink);
	}
	
}

void AAutomataFactory::DriverSetup()
{
	Driver = NewObject<UAutomataStepDriver>(GetWorld());

	if (AutomataInterfacePtr != nullptr)
	{
		Driver->SetAutomata(AutomataInterfacePtr);
	}
	
}
