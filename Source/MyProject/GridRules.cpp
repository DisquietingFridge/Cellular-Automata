#include "HexCoords.h"
#include "GridRules.h"

void UGridSpecs::SetCoords()
{
	GridCoords.Init(FIntPoint(), NumCells());
	ParallelFor(NumZCells, [&](int z)
	{
		ParallelFor(NumXCells, [&](int x)
		{
			int ID = z * NumXCells + x;
			GridCoords[ID] = { x, z };
		});
	});
}

void UGridSpecs::SetTransforms()
{
	using namespace HexCoords;
	CellTransforms.Reserve(NumCells());

	auto VectorConversion = [&](FVector2D Point)
	{
		return Offset * FVector(Point[0], 0, Point[1]);
	};

	for (int i = 0; i < NumCells(); ++i)
	{
		FIntPoint Coord = GridCoords[i];
		FVector TransformResult;
		switch (Shape)
		{
		case CellShape::Square:
			TransformResult = VectorConversion(Coord);
			break;
		case CellShape::Hex:
			TransformResult = VectorConversion(OffsetToTransform(Coord));
			break;
		}
		CellTransforms.Add(TransformResult);
	}
}

void UGridSpecs::SetAndInit(TTuple<int, int> Dims, CellShape newShape)
{
	Shape = newShape;
	Tie(NumXCells, NumZCells) = Dims;

	SetCoords();
	SetTransforms();
}


TPair<int*, int*> NeighborhoodMaker::CompAndNumFromAxis(FIntPoint& Coord, DeformedAxis Axis) const
{
	int* Component;
	int* NumAxisCells;
	
	switch (TwistedAxis)
	{
	case DeformedAxis::XAxis:
		NumAxisCells = &NumXCells;
		Component = &(Coord[0]);
		break;
	case DeformedAxis::ZAxis:
		NumAxisCells = &NumZCells;
		Component = &(Coord[1]);
		break;
	}

	return {Component, NumAxisCells};
}

void NeighborhoodMaker::ReverseAxis(FIntPoint& Coord, DeformedAxis AxisToReverse) const
{
	LoopAxis(Coord, AxisToReverse);

	int* Component;
	int* NumAxisCells;
	Tie(Component, NumAxisCells) = CompAndNumFromAxis(Coord, AxisToReverse);

	*Component = *NumAxisCells - *Component - 1;
}

void NeighborhoodMaker::LoopAxis(FIntPoint& Coord, DeformedAxis AxisToLoop) const
{
	int* Component;
	int* NumAxisCells;
	Tie(Component, NumAxisCells) = CompAndNumFromAxis(Coord, AxisToLoop);

	*Component = *Component >= 0	?	*Component % *NumAxisCells	:	*NumAxisCells - (abs(*Component) % *NumAxisCells);
}

bool NeighborhoodMaker::IsAxisTwisted(FIntPoint& Coord, DeformedAxis TwistedAxis) const
{
	int* Component;
	int* NumAxisCells;
	Tie(Component, NumAxisCells) = CompAndNumFromAxis(Coord, TwistedAxis);

	return *Component < 0	?	not ((abs(*Component) - 1) / *NumAxisCells) % 2		:	(*Component / *NumAxisCells) % 2;
}

TSharedPtr<TArray<TSet<int>>> NeighborhoodMaker::MakeNeighborhoods()
{
	using namespace HexCoords;

	// Making sure to reserve all the memory we'll need, for parallelism thread-safety
	TArray<int> MemoryDummy;
	MemoryDummy.Reserve(RelativeNeighborhood.Num());
	TArray<TArray<int>> Neighborhoods(MemoryDummy, Grid->NumCells());

	auto Shape = Grid->GetCellShape();
	TArray<FIntPoint>& GridCoords = *(Grid->GetCoords());

	ParallelFor(Grid->NumCells(), [&](int CellID)
	{
		TArray<FIntPoint> NeighborCoords = RelativeNeighborhood;
		FIntPoint& CellCoord = GridCoords[CellID];

		switch (Shape)
		{
		case (CellShape::Square):
			
			for (FIntPoint& Coord : NeighborCoords)
			{
				Coord += CellCoord;
			}
			break;

		case (CellShape::Hex):
			// Hex coordinates can only be added properly in the axial domain:
			// convert to axial, add, then convert back

			TArray<FIntPoint>& AxialNeighborCoords = NeighborCoords;

			for (FIntPoint& Coord : AxialNeighborCoords)
			{
				Coord += OffsetToAxial(CellCoord);
				Coord = AxialToOffset(Coord);
			}
			break;
		}

		MapNeighborhood(Neighborhoods[CellID],NeighborCoords);
	});

	return MakeShared<TArray<TArray<int>>>(Neighborhoods);
}
