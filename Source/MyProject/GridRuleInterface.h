#pragma once

#include "GridRuleInterface.generated.h"

UENUM()
enum class CellShape : uint8
{
	Square,
	Hex
};

UENUM()
enum class BoundGridRuleset : uint8
{
	Finite,
	Cylinder,
	Torus,
	Klein,
	CrossSurface UMETA(DisplayName = "Cross-surface"),
	Sphere
};


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

	virtual TSharedPtr<TMap<FIntPoint, int>> MapNeighborhood(TArray<TPair<FIntPoint,FIntPoint>> NeighborInfo) const
	{
		TMap<FIntPoint, int> NullMap;
		return MakeShared<TMap<FIntPoint, int>>(NullMap);
	}
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

UINTERFACE()
class UGridSpecsInterface : public UInterface
{
	GENERATED_BODY()
};

class IGridSpecsInterface
{
	GENERATED_BODY()

public:

	virtual TTuple<int,int> GetGridDimensions()
	{
		return TTuple<int, int>(0, 0);
	}

	virtual CellShape GetCellShape()
	{
		return CellShape::Square;
	}
};