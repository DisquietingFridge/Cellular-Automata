#include "GridRules.h"
#include "HexCoords.h"


void FBasicGrid::SetCoords()
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

void FBasicGrid::SetTransforms()
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

void FNeighborhoodMaker::InitRuleFunc(BoundGridRuleset Rule)
{
	//TODO: Bind remaining rules.
	switch (Rule)
	{
	case BoundGridRuleset::Torus:
		ApplyEdgeRule = &FNeighborhoodMaker::TorusRule;
		break;

	case BoundGridRuleset::Finite:
		ApplyEdgeRule = &FNeighborhoodMaker::FiniteRule;
		break;

	case BoundGridRuleset::Cylinder:
		ApplyEdgeRule = &FNeighborhoodMaker::CylinderRule;
		break;

	case BoundGridRuleset::Klein:
		ApplyEdgeRule = &FNeighborhoodMaker::KleinRule;
		break;

	case BoundGridRuleset::CrossSurface:
		ApplyEdgeRule = &FNeighborhoodMaker::CrossSurfaceRule;
		break;

	case BoundGridRuleset::Sphere:
		ApplyEdgeRule = &FNeighborhoodMaker::SphereRule;
		break;

	default:
		ApplyEdgeRule = &FNeighborhoodMaker::TorusRule;
		break;
	}
}


//TODO: Currently this implementation (adding to a set) does not allow for duplicate entries in the neighborhood. Desirable?
void FNeighborhoodMaker::MapNeighborhood(TArray<int>& Neighborhood, TArray<FIntPoint>& NeighborCoords)
{
	TSet<int> ConvertedCoords;
	for (auto Coord : NeighborCoords)
	{
		if (ApplyEdgeRule != nullptr)
		{
			ConvertedCoords.Add((this->*ApplyEdgeRule)(Coord));
		}	
	}
	ConvertedCoords.Remove(-1);

	Neighborhood = ConvertedCoords.Array();
}

void FNeighborhoodMaker::ReverseAxis(int & Component, int NumAxisCells) const
{
	LoopAxis(Component, NumAxisCells);
	Component = NumAxisCells - Component - 1;
}

void FNeighborhoodMaker::LoopAxis(int & Component, int NumAxisCells) const
{
	Component = Component >= 0 ? 
				Component % NumAxisCells : 
				NumAxisCells - ((abs(Component) - 1) % (NumAxisCells)) - 1;
}

bool FNeighborhoodMaker::IsAxisTwisted(int & Component, int NumAxisCells) const
{
	return	Component < 0 ? 
			! bool(		((abs(Component) - 1) / NumAxisCells) % 2) : 
			bool(		(Component / NumAxisCells) % 2);
}

int FNeighborhoodMaker::TorusRule(FIntPoint& Coord)
{
	int& NumXCells = Grid->NumXCells;
	int& NumZCells = Grid->NumZCells;

	int& XCoord = Coord[0];
	int& ZCoord = Coord[1];

	LoopAxis(XCoord, NumXCells);
	LoopAxis(ZCoord, NumZCells);

	return Grid->CoordToCellID(Coord);
}

int FNeighborhoodMaker::FiniteRule(FIntPoint& Coord)
{
	int& NumXCells = Grid->NumXCells;
	int& NumZCells = Grid->NumZCells;

	int& XCoord = Coord[0];
	int& ZCoord = Coord[1];

	if (((XCoord >= 0) && (XCoord < NumXCells)) && ((ZCoord >= 0) && (ZCoord < NumZCells)))
	{
		return Grid->CoordToCellID(Coord);
	}
	else
	{
		return -1;
	}
}

int FNeighborhoodMaker::CylinderRule(FIntPoint& Coord)
{
	int& NumXCells = Grid->NumXCells;
	int& NumZCells = Grid->NumZCells;

	int& XCoord = Coord[0];
	int& ZCoord = Coord[1];

	if ((ZCoord >= 0) && (ZCoord < NumZCells))
	{
		LoopAxis(XCoord, NumXCells);
		return Grid->CoordToCellID(Coord);
	}
	else
	{
		return -1;
	}
}

int FNeighborhoodMaker::KleinRule(FIntPoint& Coord)
{
	int& NumXCells = Grid->NumXCells;
	int& NumZCells = Grid->NumZCells;

	int& XCoord = Coord[0];
	int& ZCoord = Coord[1];

	if (IsAxisTwisted(ZCoord, NumZCells))
	{
		ReverseAxis(XCoord, NumXCells);
	}
	else
	{
		LoopAxis(XCoord, NumXCells);
	}

	LoopAxis(ZCoord, NumZCells);

	return Grid->CoordToCellID(Coord);
}

int FNeighborhoodMaker::CrossSurfaceRule(FIntPoint& Coord)
{
	int& NumXCells = Grid->NumXCells;
	int& NumZCells = Grid->NumZCells;

	int& XCoord = Coord[0];
	int& ZCoord = Coord[1];

	if (IsAxisTwisted(XCoord, NumXCells))
	{
		ReverseAxis(ZCoord, NumZCells);
	}
	else
	{
		LoopAxis(ZCoord, NumZCells);
	}

	if (IsAxisTwisted(ZCoord, NumZCells))
	{
		ReverseAxis(XCoord, NumXCells);
	}
	else
	{
		LoopAxis(XCoord, NumXCells);
	}

	return Grid->CoordToCellID(Coord);


}

int FNeighborhoodMaker::SphereRule(FIntPoint& Coord)
{
	int& NumXCells = Grid->NumXCells;
	int& NumZCells = Grid->NumZCells;

	int& XCoord = Coord[0];
	int& ZCoord = Coord[1];

	bool XAxisCrossed = IsAxisTwisted(XCoord, NumXCells);
	bool ZAxisCrossed = IsAxisTwisted(ZCoord, NumZCells);

	int Rotation = XAxisCrossed + 2 * ZAxisCrossed;

	// how to virtually change grid orientation (cell is considered "fixed" as grid rotates around it)
	switch (Rotation)
	{
	case 0:
		// 0 degrees
		break;

	case 1:
		// 90 degrees CW

		LoopAxis(XCoord, NumXCells);
		ReverseAxis(ZCoord, NumZCells);

		Swap(XCoord, ZCoord);

		LoopAxis(XCoord, NumXCells);
		LoopAxis(ZCoord, NumZCells);

		break;

		// X axis becomes Z axis
		// Z axis becomes -X axis

	case 2:
		// 270 degrees CW

		ReverseAxis(XCoord, NumXCells);
		LoopAxis(ZCoord, NumZCells);

		Swap(XCoord, ZCoord);

		LoopAxis(XCoord, NumXCells);
		LoopAxis(ZCoord, NumZCells);

		// X axis becomes -Z axis
		// Z axis becomes X axis

	case 3:

		//180 degrees CW

		ReverseAxis(XCoord, NumXCells);
		ReverseAxis(ZCoord, NumZCells);
		break;

		// X axis becomes -X axis
		// Z axis becomes -Z axis
	}

	return Grid->CoordToCellID(Coord);
}

void FNeighborhoodMaker::MakeNeighborhoods(TArray<TArray<int>>& Neighborhoods, TArray<FIntPoint> RelativeNeighborhood, BoundGridRuleset Rule)
{
	using namespace HexCoords;

	InitRuleFunc(Rule);

	TArray<FIntPoint>& GridCoords = Grid->GridCoords;

	// Making sure to reserve all the memory we'll need, for parallelism thread-safety

	Neighborhoods.Init(TArray<int>(), GridCoords.Num());

	ParallelFor(GridCoords.Num(), [&](int CellID)
	{
		TArray<FIntPoint> NeighborCoords = RelativeNeighborhood;
		FIntPoint& CellCoord = GridCoords[CellID];

		switch (Grid->Shape)
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
