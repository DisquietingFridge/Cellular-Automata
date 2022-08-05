#pragma once

#include "CoreMinimal.h"
#include "GridRuleInterface.h"
#include "GridEdgeRules.generated.h"

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