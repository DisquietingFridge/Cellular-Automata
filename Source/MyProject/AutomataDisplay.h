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

	TSharedRef<TArray<float>> SwitchSteps = MakeShared<TArray<float>>(TArray<float>());
	
	public:

	void SetSwitchSteps(TSharedRef<TArray<float>> RPtr)
	{
		SwitchSteps = RPtr;
	}

	void InitMaterial(UMaterialInterface* Mat, TMap<FName, float> MatScalars, TMap<FName, FLinearColor> MatVectors);
	void InitializeNiagaraSystem(UNiagaraSystem* System, USceneComponent* Root, UGridSpecs* Grid);

	void UpdateDisplay();



};