#pragma once

#include "GridRuleInterface.generated.h"

UINTERFACE()
class UGridRuleInterface : public UInterface
{
	GENERATED_BODY()
};

class IGridRuleInterface
{
	GENERATED_BODY()

public:

	virtual void InitializeGridParams(int NumXCells, int NumZCells)
	{
	}

	virtual TSharedPtr<TArray<int>> RawCoordsToCellIDs(TArray<FIntPoint>& RawCoords) const
	{
		TArray<int> NullNeighborhood;

		return MakeShared<TArray<int>>(NullNeighborhood);
	}
};

UENUM()
enum class BoundGridRuleset : uint8
{
	Finite,
	Cylinder,
	Torus,
	Klein,
	CrossSurface UMETA(DisplayName = "Cross-surface"),
	Sphere,
	Test
};

UENUM()
enum class DeformedAxis : uint8
{
	XAxis,
	ZAxis
};

UINTERFACE()
class UGridRuleFactoryInterface : public UInterface
{
	GENERATED_BODY()
};

class IGridRuleFactoryInterface
{
	GENERATED_BODY()

public:

	virtual IGridRuleInterface* CreateGridRuleInterface(BoundGridRuleset Ruleset)
	{
		return nullptr;
	}
};