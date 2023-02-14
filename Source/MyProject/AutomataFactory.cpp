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
	Driver->SetTimer(DisplayParameters.StepPeriod);
}

void AAutomataFactory::PreInitializeComponents()
{
	Super::PreInitializeComponents();

}

void AAutomataFactory::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GridSetup();
	DisplaySetup();
	RuleCalcSetup();
	DriverSetup();
}

void AAutomataFactory::GridSetup()
{
	Grid.SetCoords();
	Grid.SetTransforms();
}

TArray<FIntPoint> AAutomataFactory::GetRelativeNeighborhood()
{
	if (AutomataType == UAntRule::StaticClass())
	{
		return RelativeCardinalNeighborhood;
	}

	switch (Grid.Shape)
	{
	case CellShape::Square:
		return RelativeMooreNeighborhood;
	case CellShape::Hex:
		return RelativeAxialNeighborhood;
	default:
		return RelativeMooreNeighborhood;
	}
}

void AAutomataFactory::RuleCalcSetup()
{
	if (AutomataType != nullptr)
	{
		Automata = NewObject<UObject>(GetWorld(), AutomataType);
	}
	else
	{
		return;
	}

	AutomataInterfacePtr = Cast<IAutomata>(Automata);
	if (AutomataInterfacePtr != nullptr)
	{
		TArray<TArray<int>> Neighborhoods;
		FNeighborhoodMaker(&Grid).MakeNeighborhoods(Neighborhoods, GetRelativeNeighborhood(), SelectedGridRule);

		AutomataInterfacePtr->SetBaseMembers({Neighborhoods, Display});
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

	Display = NewObject<UAutomataDisplay>( GetWorld(), DisplayType);

	Display->InitializeNiagaraSystem(RootComponent,DisplayParameters, Grid);
}

void AAutomataFactory::DriverSetup()
{
	Driver = NewObject<UAutomataStepDriver>(GetWorld());

	if (AutomataInterfacePtr != nullptr)
	{
		Driver->SetAutomata(AutomataInterfacePtr);
	}
}
