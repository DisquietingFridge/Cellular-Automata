#pragma once

#include "GridEdgeRules.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "AutomataDriver.generated.h"





const TArray<FIntPoint> RelativeMooreNeighborhood
{
	{-1,-1}, {0,-1}, {1,-1},
	{-1,0}, {1,0},
	{-1,1}, {0,1}, {1,1}
};

const TArray<FIntPoint> RelativeAxialNeighborhood
{
	{0,-1}, {1,-1},
	{1,0}, {0,1},
	{-1,1}, {-1,0}
	/*{-2,0}, {0,-2}*/
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

	virtual void StartingDataSetup();

	virtual void SetCellNextCustomData();

	virtual void ApplyCellRules();

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

	FAsyncTask<AAutomataDriver>* Processor;

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

	// Handles automata step completion and transition into next step
	UFUNCTION()
		void StepComplete();

	// Called when StepTimer is fired
	UFUNCTION()
		void TimerFired();

	virtual void CellProcessorWork();

	TFuture<void> AsyncState;

	TFunction<void()> Work = [&]() {
		CellProcessorWork();
	};

	public:

	TTuple<int,int> GetGridDimensions() override
	{
		return TTuple<int,int>(NumXCells, NumZCells);
	}

	virtual CellShape GetCellShape() override
	{
		return Shape;
	}

};


