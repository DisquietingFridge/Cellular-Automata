#include "GridRules.h"
#include "HexCoords.h"


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
			TransformResult = VectorConversion(OffsetToTransform(Coord, OffsetLayout::OddR));
			break;
		}
		CellTransforms.Add(TransformResult);
	}
}

void UGridSpecs::SetAndInit(TTuple<int, int> Dims, float newOffset, CellShape newShape)
{
	Shape = newShape;
	Offset = newOffset;
	Tie(NumXCells, NumZCells) = Dims;

	SetCoords();
	SetTransforms();
}


void UNeighborhoodMaker::InitRuleFunc(BoundGridRuleset Rule)
{
	//TODO: Bind remaining rules.
	switch (Rule)
	{
	case BoundGridRuleset::Torus:
		ApplyEdgeRule.BindUObject(this, &UNeighborhoodMaker::TorusRule);
		break;
	
	default:
		ApplyEdgeRule.BindUObject(this, &UNeighborhoodMaker::TorusRule);
		break;
	}
}


//TODO: Currently this implementation (adding to a set) does not allow for duplicate entries in the neighborhood. Desirable?
void UNeighborhoodMaker::MapNeighborhood(TArray<int>& Neighborhood, TArray<FIntPoint>& NeighborCoords)
{
	TSet<int> ConvertedCoords;
	for (auto Coord : NeighborCoords)
	{
		if (ApplyEdgeRule.IsBound())
		{
			ConvertedCoords.Add(ApplyEdgeRule.Execute(Coord));
		}	
	}
	ConvertedCoords.Remove(-1);

	Neighborhood = ConvertedCoords.Array();
}

TPair<int*, const int*> UNeighborhoodMaker::CompAndNumFromAxis(FIntPoint& Coord, DeformedAxis Axis) const
{
	int* Component;
	const int* NumAxisCells;
	
	switch (Axis)
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

	return TPair<int*, const int*>(Component, NumAxisCells);
}

void UNeighborhoodMaker::ReverseAxis(FIntPoint& Coord, DeformedAxis AxisToReverse) const
{
	LoopAxis(Coord, AxisToReverse);

	int* Component;
	const int* NumAxisCells;
	Tie(Component, NumAxisCells) = CompAndNumFromAxis(Coord, AxisToReverse);

	*Component = *NumAxisCells - *Component - 1;
}

void UNeighborhoodMaker::LoopAxis(FIntPoint& Coord, DeformedAxis AxisToLoop) const
{
	int* Component;
	const int* NumAxisCells;
	Tie(Component, NumAxisCells) = CompAndNumFromAxis(Coord, AxisToLoop);

	//int& ComponentDeref = *Component;
	//const int& NumAxisCellsDeref = *NumAxisCells;

	*Component = *Component >= 0 ? *Component % *NumAxisCells : *NumAxisCells - ((abs(*Component) - 1) % (*NumAxisCells)) - 1;
}

bool UNeighborhoodMaker::IsAxisTwisted(FIntPoint& Coord, DeformedAxis TwistedAxis) const
{
	int* Component;
	const int* NumAxisCells;
	Tie(Component, NumAxisCells) = CompAndNumFromAxis(Coord, TwistedAxis);

	return *Component < 0 ? 
			! bool(		((abs(*Component) - 1) / *NumAxisCells) % 2) : 
			bool(		(*Component / *NumAxisCells) % 2);
}

int UNeighborhoodMaker::TorusRule(FIntPoint& Coord)
{
	LoopAxis(Coord, DeformedAxis::XAxis);
	LoopAxis(Coord, DeformedAxis::ZAxis);

	return CoordToCellID(Coord);
}

void UNeighborhoodMaker::MakeNeighborhoods(TArray<TArray<int>>& Neighborhoods, TArray<FIntPoint> RelativeNeighborhood, BoundGridRuleset Rule)
{
	using namespace HexCoords;

	InitRuleFunc(Rule);

	auto Shape = Grid->GetCellShape();
	TArray<FIntPoint>& GridCoords = *(Grid->GetCoords());

	// Making sure to reserve all the memory we'll need, for parallelism thread-safety

	//TODO: It's quite likely memory reservation does not work this way. Fix if needed (ditching parallelism as last resort)
	Neighborhoods.Init(TArray<int>(), GridCoords.Num());

	ParallelFor(GridCoords.Num(), [&](int CellID)
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
	}/*,EParallelForFlags::ForceSingleThread*/);
}

void UNeighborhoodMaker::MakeNeighborsOf(TArray<TArray<int>>& NeighborsOf, TArray<TArray<int>>& Neighborhoods)
{
	TArray<int> MemoryDummy;
	NeighborsOf.Init(MemoryDummy, Neighborhoods.Num());

	TSet<int> SetDummy;
	TArray<TSet<int>> DupeGuard;
	DupeGuard.Init(SetDummy, NeighborsOf.Num());

	for (int i = 0; i < NeighborsOf.Num(); ++i)
	{
		for (int Neighbor : Neighborhoods[i])
		{
			DupeGuard[Neighbor].Add(i);
		}
	}

	for (int i = 0; i < NeighborsOf.Num(); ++i)
	{
		NeighborsOf[i] = DupeGuard[i].Array();
	}

}
