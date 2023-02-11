#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridRules.h"
#include "AutomataFactory.generated.h"

class UNiagaraSystem;
class UGridSpecs;
class ULifelikeRule;
class UAutomataDisplay;
class UAutomataStepDriver;
class IAutomata;

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

const TArray<FIntPoint> RelativeCardinalNeighborhood
{
	{0,1}, {1,0}, {0, -1}, {-1,0}
};

UCLASS()
class MYPROJECT_API AAutomataFactory : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AAutomataFactory();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;

	void GridSetup();

	TArray<FIntPoint> GetRelativeNeighborhood();
	void RuleCalcSetup();
	void DisplaySetup();
	void DriverSetup();

	UPROPERTY(Blueprintable, EditAnywhere)
	FBasicGrid Grid;

	UPROPERTY()
	UObject* Automata = nullptr;

	IAutomata* AutomataInterfacePtr = nullptr;

	UPROPERTY()
	UAutomataDisplay* Display = nullptr;

	UPROPERTY()
	UAutomataStepDriver* Driver = nullptr;

	UPROPERTY(Blueprintable, EditAnywhere, meta = (MustImplement = "Automata"))
		UClass* AutomataType;

	UPROPERTY(Blueprintable, EditAnywhere)
		UNiagaraSystem* ParticleSystem = nullptr;

	// Mesh that will be instanced to form the grid- typically a simple square
	UPROPERTY(Blueprintable, EditAnywhere)
		UStaticMesh* Mesh;

	// Material that will be instanced and applied to the mesh.
	// This needs to be specifically made for automata in order for it to display anything interesting
	UPROPERTY(Blueprintable, EditAnywhere)
		UMaterialInterface* Mat;

	UPROPERTY(Blueprintable, EditAnywhere)
		BoundGridRuleset SelectedGridRule = BoundGridRuleset::Finite;

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

	// time per automata step in seconds
	UPROPERTY(Blueprintable, EditAnywhere)
		float StepPeriod = 0.01;

	// Exponent that drives how quickly a switched-off cell fades into the off state
	// An exponent of 1 will fade linearly over the transition period. A higher exponent will fade out quicker initially, and a lower exponent will fade out slower initially.
	UPROPERTY(Blueprintable, EditAnywhere)
		float PhaseExponent = 201;

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

public:
};
