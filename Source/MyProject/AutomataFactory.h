#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridRules.h"
#include "AutomataDisplay.h"
#include "AutomataFactory.generated.h"

class UNiagaraSystem;
class UGridSpecs;
class ULifelikeRule;
class UAutomataDisplay;
class UAutomataStepDriver;
class IAutomata;
struct FDisplayMembers;

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

	UPROPERTY(Blueprintable, EditAnywhere, meta = (MustImplement = "Automata"))
	UClass* AutomataType;

	UPROPERTY(Blueprintable, EditAnywhere)
	TSubclassOf<UAutomataDisplay> DisplayType;

	UPROPERTY(Blueprintable, EditAnywhere)
	FBasicGrid Grid;

	UPROPERTY(Blueprintable, EditAnywhere)
		FDisplayMembers DisplayParameters;

	UPROPERTY()
	UObject* Automata = nullptr;

	IAutomata* AutomataInterfacePtr = nullptr;

	UPROPERTY()
	UAutomataDisplay* Display = nullptr;

	UPROPERTY()
	UAutomataStepDriver* Driver = nullptr;

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

	



	



public:
};
