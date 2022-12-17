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

UENUM()
enum class DeformedAxis : uint8
{
	XAxis,
	ZAxis
};


//UCLASS()
//class UGridRuleInterface : public UObject
//{
//	GENERATED_BODY()
//
//public:
//
//	virtual void InitializeGridParams(int NumXCells, int NumZCells)
//	{
//	}
//
//	virtual TSharedPtr<TArray<int>> RawCoordsToCellIDs(TArray<FIntPoint>& RawCoords) const
//	{
//		TArray<int> NullNeighborhood;
//
//		return MakeShared<TArray<int>>(NullNeighborhood);
//	}
//
//	virtual TSharedPtr<TMap<FIntPoint, int>> MapNeighborhood(TArray<TPair<FIntPoint,FIntPoint>> NeighborInfo) const
//	{
//		TMap<FIntPoint, int> NullMap;
//		return MakeShared<TMap<FIntPoint, int>>(NullMap);
//	}
//};


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

	TSharedPtr<TArray<FIntPoint>> GetCoords()
	{
		return MakeShared<TArray<FIntPoint>>(GridCoords);
	}

	TSharedPtr<TArray<FVector>> GetTransforms()
	{
		return MakeShared<TArray<FVector>>(CellTransforms);
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

	TPair<int*, const int*> CompAndNumFromAxis(FIntPoint& Coord, DeformedAxis Axis) const;

	int CoordToCellID(FIntPoint Coord) const
	{
		return Coord[1] * NumXCells		+ Coord[0];
	}

	void ReverseAxis(FIntPoint& Coord, DeformedAxis AxisToReverse) const;

	void LoopAxis(FIntPoint& Coord, DeformedAxis AxisToLoop) const;

	bool IsAxisTwisted(FIntPoint& Coord, DeformedAxis TwistedAxis) const;

	int TorusRule(FIntPoint& Coord);

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