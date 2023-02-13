#pragma once

#include "AutomataDisplay.generated.h"


struct FBasicGrid;
class UNiagaraSystem;
class UNiagaraComponent;

USTRUCT()
struct FDisplayMembers
{

	GENERATED_BODY()
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
		TArray<FLinearColor> OtherColors = { FLinearColor(0.0, 0, 0.0, 1) };

	// Material property used to control emissive value
	UPROPERTY(Blueprintable, EditAnywhere)
		float EmissiveMultiplier = 20;

	// how many automata steps a dead cell takes to fade out after death
	UPROPERTY(Blueprintable, EditAnywhere)
		float StepsToFade = 1000;

	TMap<FName, float> MatFloats();

};

UCLASS(Blueprintable)
class UAutomataDisplay : public UObject
{
	GENERATED_BODY()

	private:

	UPROPERTY(Blueprintable, EditAnywhere)
	UNiagaraSystem* ParticleSystem;

	// Material that will be instanced and applied to the mesh.
// This needs to be specifically made for automata in order for it to display anything interesting
	UPROPERTY(Blueprintable, EditAnywhere)
	UMaterialInterface* Mat;

	//UMaterialInstanceDynamic* DynMaterial;

	UNiagaraComponent* NiagaraComponent = nullptr;
	
	UMaterialInstanceDynamic* MakeMaterial(FDisplayMembers& DisplayParams, const FBasicGrid& Grid);

	public:

	
	void InitializeNiagaraSystem(USceneComponent* Root, FDisplayMembers& DisplayParams, const FBasicGrid& Grid);

	void UpdateDisplay(const TArray<float> & SwitchSteps);
};