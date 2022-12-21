#pragma once

#include "AutomataDisplay.generated.h"


class UGridSpecs;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class UAutomataDisplay : public UObject
{
	GENERATED_BODY()

	private:

	UPROPERTY(Blueprintable, EditAnywhere)
	UNiagaraSystem* ParticleSystem;

	UMaterialInstanceDynamic* DynMaterial;

	UNiagaraComponent* NiagaraComponent = nullptr;
	
	public:

	void InitMaterial(UMaterialInterface* Mat, TMap<FName, float> MatScalars, TMap<FName, FLinearColor> MatVectors);
	void InitializeNiagaraSystem(UNiagaraSystem* System, USceneComponent* Root, UGridSpecs* Grid);

	void UpdateDisplay(const TArray<float> & SwitchSteps);



};