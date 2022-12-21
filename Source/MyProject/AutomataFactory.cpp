#include "AutomataFactory.h"
#include "GridRules.h"
#include "Rulesets.h"
#include "AutomataDisplay.h"
#include "AutomataStepDriver.h"

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
	Lifelike->BroadcastData();
	Lifelike->StartNewStep();
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

void AAutomataFactory::RuleCalcSetup()
{
	Lifelike = NewObject<ULifelikeRule>(this);

	Lifelike->SetNeighborhoods(Neighborhoods);
	Lifelike->SetNeighborsOf(Neighborhoods);
	Lifelike->InitializeCellRules(BirthString, SurviveString);
	Lifelike->InitializeCellStates(Probability);

	Lifelike->StartingDataSetup();
}

void AAutomataFactory::DisplaySetup()
{
	if (ParticleSystem == nullptr)
	{
		return;
	}

	Display = NewObject<UAutomataDisplay>(this);

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

	ULifelikeRule::SendDisplayData DisplayLink;
	DisplayLink.AddUObject(Display, &UAutomataDisplay::UpdateDisplay);
	Lifelike->SetBroadcast(DisplayLink);
}

void AAutomataFactory::DriverSetup()
{
	Driver = NewObject<UAutomataStepDriver>(this);

	UAutomataStepDriver::DriverStepEvent WaitAndFinalize;
	WaitAndFinalize.AddUObject(Lifelike, &ULifelikeRule::StepComplete);

	UAutomataStepDriver::DriverStepEvent UpdateDisplay;
	UpdateDisplay.AddUObject(Lifelike, &ULifelikeRule::BroadcastData);

	UAutomataStepDriver::DriverStepEvent NewStep;
	NewStep.AddUObject(Lifelike, &ULifelikeRule::StartNewStep);

	Driver->SetEvents(WaitAndFinalize, UpdateDisplay, NewStep);
}



// Called every frame
void AAutomataFactory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
