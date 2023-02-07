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


UCLASS()
class UGridSpecs : public UObject
{
	GENERATED_BODY()

private:

	UPROPERTY(Blueprintable, EditAnywhere)
		CellShape Shape = CellShape::Square;

	UPROPERTY(Blueprintable, EditAnywhere)
		int NumXCells = 100;

	UPROPERTY(Blueprintable, EditAnywhere)
		int NumZCells = 100;

	UPROPERTY(Blueprintable, EditAnywhere)
		float Offset = 1;

	TArray<FIntPoint> GridCoords;
	TArray<FVector> CellTransforms;

	void SetCoords();
	void SetTransforms();

public:

	void SetAndInit(TTuple<int, int> Dims, float newOffset, CellShape newShape);

	TTuple<int,int> GetGridDimensions()
	{
		return TTuple<int, int>(NumXCells, NumZCells);
	}

	void SetGridDimensions(TTuple<int, int> Dims)
	{
		Tie(NumXCells, NumZCells) = Dims;
	}

	CellShape GetCellShape()
	{
		return Shape;
	}

	void SetCellShape(CellShape newShape)
	{
		this->Shape = newShape;
	}

	int NumCells()
	{
		return NumXCells * NumZCells;
	}

	TArray<FIntPoint>* GetCoords()
	{
		return &GridCoords;
	}

	TArray<FVector>* GetTransforms()
	{
		return &CellTransforms;
	}

};

UCLASS()
class UNeighborhoodMaker : public UObject
{
	GENERATED_BODY()

	DECLARE_DELEGATE_RetVal_OneParam(int, CoordConverter, FIntPoint&);

private:

	CoordConverter ApplyEdgeRule;

	int NumXCells = 0;
	int NumZCells = 0;

	UGridSpecs* Grid = nullptr;

	void MapNeighborhood(TArray<int>& Neighborhood, TArray<FIntPoint>& NeighborCoords);

	int CoordToCellID(FIntPoint Coord) const
	{
		return Coord[1] * NumXCells		+ Coord[0];
	}

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

	void Initialize(UGridSpecs* GridSpecs)
	{
		this->Grid = GridSpecs;
		Tie(NumXCells, NumZCells) = Grid->GetGridDimensions();
	}


	void MakeNeighborhoods(TArray<TArray<int>>& Neighborhoods, TArray<FIntPoint> RelativeNeighborhood, BoundGridRuleset Rule);
	void MakeNeighborsOf(TArray<TArray<int>>& NeighborsOf, TArray<TArray<int>>& Neighborhoods);

};