#include "AutomataDisplay.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

#include "GridRules.h"

typedef UNiagaraDataInterfaceArrayFunctionLibrary NiagaraFuncs;

void UAutomataDisplay::InitMaterial(UMaterialInterface* Mat, TMap<FName, float> MatScalars, TMap<FName, FLinearColor> MatVectors)
{
	DynMaterial = UMaterialInstanceDynamic::Create(Mat, this);

	for (const auto& NameScalar : MatScalars)
	{
		DynMaterial->SetScalarParameterValue(NameScalar.Key, NameScalar.Value);
	}

	for (const auto& NameVec : MatVectors)
	{
		DynMaterial->SetVectorParameterValue(NameVec.Key, NameVec.Value);
	}

	/*DynMaterial->SetScalarParameterValue("PhaseExponent", PhaseExponent);
	DynMaterial->SetScalarParameterValue("EmissiveMultiplier", EmissiveMultiplier);
	DynMaterial->SetVectorParameterValue("OnColor", OnColor);
	DynMaterial->SetVectorParameterValue("OffColor", OffColor);
	DynMaterial->SetScalarParameterValue("FadePerSecond", 1 / (StepPeriod * StepsToFade));

	DynMaterial->SetScalarParameterValue("IsHexagon", Shape == CellShape::Hex);*/
}

//TODO: Make this spawn at location, not attached to a root
void UAutomataDisplay::InitializeNiagaraSystem(UNiagaraSystem* System, USceneComponent* Root, UGridSpecs* Grid)
{
	ParticleSystem = System;
	// TODO: Make sure this is paranted properly
	NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(ParticleSystem, Root, FName(), FVector(0), FRotator(0), EAttachLocation::KeepRelativeOffset, false, false, ENCPoolMethod::None, true);

	auto& Transforms = *(Grid->GetTransforms());

	NiagaraFuncs::SetNiagaraArrayVector(NiagaraComponent, "User.Transforms", Transforms);
	NiagaraComponent->SetVariableMaterial(FName("User.Material"), DynMaterial);


	int NumXCells;
	int NumZCells;
	Tie(NumXCells, NumZCells) = Grid->GetGridDimensions();

	NiagaraComponent->SetVariableInt(FName("User.XCount"), NumXCells);
	NiagaraComponent->SetVariableInt(FName("User.ZCount"), NumZCells);

	NiagaraComponent->ActivateSystem();
}

void UAutomataDisplay::UpdateDisplay()
{
	//TODO: Make sure material / Niagara system accepts SwitchSteps instead of time-domain "SwitchTimes"
	//TArray<float>& Dereffed = ;

	NiagaraFuncs::SetNiagaraArrayFloat(NiagaraComponent, "User.SwitchSteps", *SwitchSteps);

	
}
