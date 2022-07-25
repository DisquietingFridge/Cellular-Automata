#pragma once

#include "CoreMinimal.h"
#include "GridRuleInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "AutomataDriver.generated.h"


// CellProcessor is used to asynchronously update the cell states that it is responsible for,
// so that these calculations don't cause a performance bottleneck by being carried out all at once

// Asynchtask management in UE4 is poorly documented. There are seemingly many different kinds of manager types, and their benefits/tradeoffs are not clear to me.
// Important elements of getting it to work are
// 1. that the class inherits from FNonAbandonableTask,
// 2. That it be a friend class of the FAsyncTask (other kinds of async classes may be possible)
// 3. the GetStatId() function definition boilerplate needs to be in place


class CellProcessor : public FNonAbandonableTask
{
	friend class FAsyncTask<CellProcessor>;

public:

	// Initialization largely consists of getting pointers from the Driver
	CellProcessor(AAutomataDriver* Driver, TArray<int> CellIDs);


protected:

	// AutomataDriver this processor is working for
	class AAutomataDriver* Driver = nullptr;

	UPROPERTY()
		TArray<int> CellIDs;


public:

	// FAsyncTask boilerplate function, defining the task it works on when called
	// In this case: calculating updated cell states
	void DoWork();

	//FAsyncTask boilerplate. Do not remove
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

UCLASS()
class UStandardXZGrid : public UObject, public IGridRuleInterface
{
	GENERATED_BODY()

protected:
	int NumXCells;
	int NumZCells;

public:

	UStandardXZGrid();

	virtual int ApplyRule(FIntPoint Coord) const;

	virtual TSharedPtr<TArray<int>> RawCoordsToCellIDs(TArray<FIntPoint>& RawCoords) const override;

	virtual TSharedPtr<TMap<FIntPoint, int>> MapNeighborhood(TArray<TPair<FIntPoint, FIntPoint>> NeighborInfo) const override;

	bool IsAxisTwisted(FIntPoint Coord, DeformedAxis TwistedAxis) const;

	void ParamsFromAxis(int& Index, int& NumAxisCells, DeformedAxis Axis) const;

	virtual void LoopAxis(FIntPoint& Coord, DeformedAxis AxisToLoop) const;

	virtual void ReverseAxis(FIntPoint& Coord, DeformedAxis AxisToReverse) const;

	int CoordToCellID(FIntPoint Coord) const
	{
		return ((Coord[1] * NumXCells) + Coord[0]);
	}
};

UCLASS()
class USphereRule : public UStandardXZGrid
{
	GENERATED_BODY()

public:

	virtual int ApplyRule(FIntPoint Coord) const override;
};

UCLASS()
class UCrossSurfaceRule : public UStandardXZGrid
{
	GENERATED_BODY()

public:

	virtual int ApplyRule(FIntPoint Coord) const override;
};

UCLASS()
class UKleinRule : public UStandardXZGrid
{
	GENERATED_BODY()

public:

	virtual int ApplyRule(FIntPoint Coord) const override;

};

UCLASS()
class UTorusRule : public UStandardXZGrid
{
	GENERATED_BODY()

public:

	virtual int ApplyRule(FIntPoint Coord) const override;

};

UCLASS()
class UCylinderRule : public UStandardXZGrid
{
	GENERATED_BODY()

public:

	virtual int ApplyRule(FIntPoint Coord) const override;
};

UCLASS()
class UFiniteRule : public UStandardXZGrid
{
	GENERATED_BODY()

public:

	virtual int ApplyRule(FIntPoint Coord) const override;
};

const TMap<BoundGridRuleset, TSubclassOf<UStandardXZGrid>> RuleEnumToClass
= {
	{BoundGridRuleset::Finite, UFiniteRule::StaticClass()},
	{BoundGridRuleset::Cylinder, UCylinderRule::StaticClass()},
	{BoundGridRuleset::Torus, UTorusRule::StaticClass()},
	{BoundGridRuleset::Klein, UKleinRule::StaticClass()},
	{BoundGridRuleset::CrossSurface, UCrossSurfaceRule::StaticClass()},
	{BoundGridRuleset::Sphere, USphereRule::StaticClass()}
};

UCLASS()

class UBaseGridRuleFactory : public UObject, public IGridRuleFactoryInterface
{
	GENERATED_BODY()

public:
	virtual IGridRuleInterface* CreateGridRuleInterface(BoundGridRuleset Ruleset) override;
};


const TArray<FIntPoint> RelativeMooreNeighborhood
{
	FIntPoint(-1,-1), FIntPoint(0,-1), FIntPoint(1,-1),
	FIntPoint(-1,0), FIntPoint(1,0),
	FIntPoint(-1,1), FIntPoint(0,1), FIntPoint(1,1)
};

const TArray<FIntPoint> RelativeAxialNeighborhood
{
	FIntPoint(0,-1), FIntPoint(1,-1),
	FIntPoint(1,0), FIntPoint(0,1),
	FIntPoint(-1,1), FIntPoint(-1,0),
	/*FIntPoint(-2,0), FIntPoint(0,-2)*/
};



UCLASS()
class AAutomataDriver : public AActor, public IGridSpecsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAutomataDriver();

protected:

	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	virtual void SetGridCoords();

	virtual void SetRelativeNeighborhood();

	virtual void InitializeMaterial();

	virtual void InitializeCellTransforms();

	virtual void InitializeNiagaraSystem();

	virtual void InitializeCellRules();

	virtual void InitializeCellStates();

	virtual void CreateGridRuleInterface();

	virtual void InitializeCellNeighborhoods();

	virtual void InitializeCellNeighborsOf();

	virtual void InitializeCellProcessors();

	virtual void StartingDataSetup();

	virtual void SetCellNextCustomData(const TArray<int>& CellIDs);
	virtual void SetCellNextCustomData(const int CellID);

	virtual void ApplyCellRules(const TArray<int>& CellIDs);
	virtual void ApplyCellRules(const int CellID);

	virtual void TimestepPropertyShift();

	virtual int GetCellAliveNeighbors(int CellID) const;



	TArray<FVector> CellTransforms;

	UPROPERTY(Blueprintable, EditAnywhere)
		UNiagaraSystem* ParticleSystem;

	UNiagaraComponent* NiagaraComponent;

	TArray<float> SwitchTimeBuffer;

	//Set that stores the birth rules for the automata
	TArray<bool> BirthRules;
	//Set that stores the survival rules for the automata
	TArray<bool> SurviveRules;

	TArray<bool> CurrentStates;
	TArray<bool> NextStates;

	TArray<bool> EvalFlaggedThisStep;
	TArray<bool> EvalFlaggedLastStep;

	TArray<TSharedPtr<TMap<FIntPoint,int>>> Neighborhoods;
	TArray<TSharedPtr<TSet<int>>> NeighborsOf;

	TArray<FIntPoint> GridCoords;

	TSubclassOf<class UBaseGridRuleFactory> GridRuleFactoryType = UBaseGridRuleFactory::StaticClass();
	UBaseGridRuleFactory* GridRuleFactory;

	// Array of CellProcessor objects that are responsible for incrementing an associated instance collection
	TArray<FAsyncTask<CellProcessor>*> Processors;

	// Mesh that will be instanced to form the grid- typically a simple square
	UPROPERTY(Blueprintable, EditAnywhere)
		UStaticMesh* Mesh;

	// Material that will be instanced and applied to the mesh.
	// This needs to be specifically made for automata in order for it to display anything interesting
	UPROPERTY(Blueprintable, EditAnywhere)
		UMaterialInterface* Mat;

	// Dynamic material (will use Mat as its basis)
	UMaterialInstanceDynamic* DynMaterial;

	UPROPERTY(Blueprintable, EditAnywhere)
		int NumXCells UMETA(DisplayName = "# cells wide") = 100;

	UPROPERTY(Blueprintable, EditAnywhere)
		int NumZCells UMETA(DisplayName = "# cells tall") = 100;

	virtual int NumCells()
	{
		return NumXCells * NumZCells;
	}

	TArray<FIntPoint> RelativeNeighborhood = RelativeMooreNeighborhood;

	IGridRuleInterface* GridRule;

	UPROPERTY(Blueprintable, EditAnywhere)
		BoundGridRuleset SelectedGridRule = BoundGridRuleset::Torus;

	UPROPERTY(Blueprintable, EditAnywhere)
		CellShape Shape = CellShape::Hex;

	// Probability when initializing that a cell will start off alive.
	// Functionally ranges from 0 to 1.
	UPROPERTY(Blueprintable, EditAnywhere)
		float Probability = 0.4;

	// User-set string that defines the birth rules for the automata
	// Capable of accepting non-digit characters, but they will be ignored
	UPROPERTY(Blueprintable, EditAnywhere)
		FString BirthString = TEXT("3");

	// User-set string that defines the survival rules for the automata
	// Capable of accepting non-digit characters, but they will be ignored
	UPROPERTY(Blueprintable, EditAnywhere)
		FString SurviveString = TEXT("23");

	// Spacing that determines how far adjacent clusters should be placed away from each other
	// The square mesh used has a 200x200 unit area.
	UPROPERTY(Blueprintable, EditAnywhere)
		float Offset = 1;

	// time per automata step in seconds
	UPROPERTY(Blueprintable, EditAnywhere)
		float StepPeriod = 0.01;

	// Exponent that drives how quickly a switched-off cell fades into the off state
	// An exponent of 1 will fade linearly over the transition period. A higher exponent will fade out quicker initially, and a lower exponent will fade out slower initially.
	UPROPERTY(Blueprintable, EditAnywhere)
		float PhaseExponent = 201;

	// Simple float used to store the time of the next step transition
	float NextStepTime = 0;

	// "On" state cell color
	UPROPERTY(Blueprintable, EditAnywhere)
		FLinearColor OnColor = FLinearColor(0.6, 0, 0.6, 1);

	// "Off" state cell color
	UPROPERTY(Blueprintable, EditAnywhere)
		FLinearColor OffColor = FLinearColor(0.0, 0, 0.0, 1);

	// Material property used to control emissive value
	UPROPERTY(Blueprintable, EditAnywhere)
		float EmissiveMultiplier = 20;

	// how many automata steps a dead cell takes to fade out after death
	UPROPERTY(Blueprintable, EditAnywhere)
		float StepsToFade = 1000;

	// Timer that fires once for each instance collection, and one additional time to signal the end of an automata step
	FTimerHandle StepTimer;
	FTimerHandle InstanceUpdateTimer;

	// Handles automata step completion and transition into next step
	UFUNCTION()
		void StepComplete();

	// Called when StepTimer is fired
	UFUNCTION()
		void TimerFired();

public:

	virtual void CellProcessorWork(const TArray<int>& CellIDs);

	TTuple<int,int> GetGridDimensions() override
	{
		return TTuple<int,int>(NumXCells, NumZCells);
	}

	virtual CellShape GetCellShape() override
	{
		return Shape;
	}

};


