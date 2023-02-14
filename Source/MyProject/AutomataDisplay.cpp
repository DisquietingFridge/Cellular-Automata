#include "AutomataDisplay.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

#include "GridRules.h"

typedef UNiagaraDataInterfaceArrayFunctionLibrary NiagaraFuncs;

UMaterialInstanceDynamic* UAutomataDisplay::MakeMaterial(FDisplayMembers& DisplayParams, const FBasicGrid& Grid)
{
	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Mat, this);

	DynMaterial->SetVectorParameterValue("OnColor", DisplayParams.OnColor);
	DynMaterial->SetScalarParameterValue("IsHexagon", float(Grid.Shape == CellShape::Hex));
	for (const auto& NameVec : DisplayParams.MatFloats())
	{
		DynMaterial->SetScalarParameterValue(NameVec.Key, NameVec.Value);
	}

	return DynMaterial;	
}

//TODO: Make this spawn at location, not attached to a root
void UAutomataDisplay::InitializeNiagaraSystem(USceneComponent* Root, FDisplayMembers& DisplayParams, const FBasicGrid& Grid)
{
	
	// TODO: Make sure this is parented properly
	NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(ParticleSystem, Root, FName(), FVector(0), FRotator(0), EAttachLocation::KeepRelativeOffset, false, false, ENCPoolMethod::None, true);

	NiagaraFuncs::SetNiagaraArrayVector(NiagaraComponent, "User.Transforms", Grid.CellTransforms);
	NiagaraFuncs::SetNiagaraArrayColor(NiagaraComponent, "User.State Colors", DisplayParams.OtherColors);
	NiagaraComponent->SetVariableMaterial(FName("User.Material"), MakeMaterial(DisplayParams,Grid));

	NiagaraComponent->ActivateSystem();
}

void UAutomataDisplay::UpdateSwitchTimes(const TArray<float>& SwitchSteps)
{
	NiagaraFuncs::SetNiagaraArrayFloat(NiagaraComponent, "User.SwitchSteps", SwitchSteps);
}

void UAutomataDisplay::UpdateEndFadeState(const TArray<int>& EndFadeStates)
{
	NiagaraFuncs::SetNiagaraArrayInt32(NiagaraComponent, "User.End States", EndFadeStates);
}

TMap<FName, float> FDisplayMembers::MatFloats()
{
	TMap<FName, float> FloatMap;

	FloatMap.Add("StepPeriod", StepPeriod);
	FloatMap.Add("PhaseExponent", PhaseExponent);
	FloatMap.Add("EmissiveMultiplier", EmissiveMultiplier);
	FloatMap.Add("FadePerSecond", 1 / (StepPeriod * StepsToFade));

	return FloatMap;
}
