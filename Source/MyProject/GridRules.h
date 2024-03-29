#pragma once

#include "GridRules.generated.h"

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

USTRUCT(Blueprintable)
struct FBasicGrid
{
	GENERATED_BODY()

	UPROPERTY(Blueprintable, EditAnywhere)
		CellShape Shape = CellShape::Square;

	UPROPERTY(Blueprintable, EditAnywhere)
		int NumXCells = 100;

	UPROPERTY(Blueprintable, EditAnywhere)
		int NumZCells = 100;

	// Spacing that determines how far adjacent cells should be placed away from each other
	UPROPERTY(Blueprintable, EditAnywhere)
		float Offset = 1;

	TArray<FIntPoint> GridCoords;
	TArray<FVector> CellTransforms;

	void SetCoords();
	void SetTransforms();

	int NumCells()
	{
		return NumXCells * NumZCells;
	}

	int CoordToCellID(FIntPoint Coord) const
	{
		return Coord[1] * NumXCells + Coord[0];
	}
};

USTRUCT()
struct FNeighborhoodMaker
{
	GENERATED_BODY()

		//DECLARE_DELEGATE_RetVal_OneParam(int, CoordConverter, FIntPoint&);
		typedef int (FNeighborhoodMaker::* RulePtr)(FIntPoint&);

private:

	FBasicGrid* Grid = nullptr;
	RulePtr ApplyEdgeRule = nullptr;

	void MapNeighborhood(TArray<int>& Neighborhood, TArray<FIntPoint>& NeighborCoords);

	void ReverseAxis(int & Component, int NumAxisCells) const;

	void LoopAxis(int & Component, int NumAxisCells) const;

	bool IsAxisTwisted(int & Component, int NumAxisCells) const;

	int TorusRule(FIntPoint& Coord);

	int FiniteRule(FIntPoint& Coord);

	int CylinderRule(FIntPoint& Coord);

	int KleinRule(FIntPoint& Coord);

	int CrossSurfaceRule(FIntPoint& Coord);

	int SphereRule(FIntPoint& Coord);

	void InitRuleFunc(BoundGridRuleset Rule);

public:

	FNeighborhoodMaker() {}

	FNeighborhoodMaker(FBasicGrid* initGrid)
	{
		Grid = initGrid;
	}

	void MakeNeighborhoods(TArray<TArray<int>>& Neighborhoods, TArray<FIntPoint> RelativeNeighborhood, BoundGridRuleset Rule);
};