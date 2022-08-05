#include "GridEdgeRules.h"

UStandardXZGrid::UStandardXZGrid()
{
	IGridSpecsInterface* Grid = Cast<IGridSpecsInterface>(GetOuter());
	if (Grid)
	{
		Tie(NumXCells, NumZCells) = Grid->GetGridDimensions();
	}
}

int UStandardXZGrid::ApplyRule(FIntPoint Coord) const
{
	return -1;
}

TSharedPtr<TArray<int>> UStandardXZGrid::RawCoordsToCellIDs(TArray<FIntPoint>& RawCoords) const
{
	TArray<int> CellIDs;
	for (FIntPoint Coord : RawCoords)
	{
		int CellID = ApplyRule(Coord);
		if (CellID >= 0)
		{
			CellIDs.Add(CellID);
		}
	}
	TSharedPtr<TArray<int>> Pointer = MakeShared<TArray<int>>(CellIDs);
	return Pointer;
}

TSharedPtr<TMap<FIntPoint, int>> UStandardXZGrid::MapNeighborhood(TArray<TPair<FIntPoint, FIntPoint>> NeighborInfo) const
{
	TMap<FIntPoint, int> NeighborhoodMap;

	for (int i = 0; i < NeighborInfo.Num(); ++i)
	{
		FIntPoint RawCoord = NeighborInfo[i].Value;
		int CellID = ApplyRule(RawCoord);
		if (CellID >= 0)
		{
			FIntPoint Key = NeighborInfo[i].Key;
			NeighborhoodMap.Add(Key, CellID);
		}
	}
	TSharedPtr<TMap<FIntPoint, int>> Pointer = MakeShared<TMap<FIntPoint, int>>(NeighborhoodMap);
	return Pointer;
}

bool UStandardXZGrid::IsAxisTwisted(FIntPoint Coord, DeformedAxis TwistedAxis) const
{
	int NumAxisCells = 0;
	int Index = 0;

	switch (TwistedAxis)
	{
	case DeformedAxis::XAxis:
		NumAxisCells = NumXCells;
		Index = 0;
		break;
	case DeformedAxis::ZAxis:
		NumAxisCells = NumZCells;
		Index = 1;
		break;
	}

	if (Coord[Index] < 0)
	{
		return !(bool)(((abs(Coord[Index]) - 1) / NumAxisCells) % 2);
	}
	else
	{
		return bool((Coord[Index] / NumAxisCells) % 2);
	}
}

void UStandardXZGrid::ParamsFromAxis(int& Index, int& NumAxisCells, DeformedAxis Axis) const
{
	switch (Axis)
	{
	case DeformedAxis::XAxis:
		NumAxisCells = NumXCells;
		Index = 0;
		break;
	case DeformedAxis::ZAxis:
		NumAxisCells = NumZCells;
		Index = 1;
		break;
	}
}

void UStandardXZGrid::LoopAxis(FIntPoint& Coord, DeformedAxis AxisToLoop) const
{
	int NumAxisCells;
	int Index;
	ParamsFromAxis(Index, NumAxisCells, AxisToLoop);

	if (Coord[Index] >= 0)
	{
		Coord[Index] = Coord[Index] % NumAxisCells;
	}
	else
	{
		Coord[Index] = NumAxisCells - (abs(Coord[Index]) % NumAxisCells);
	}
}

void UStandardXZGrid::ReverseAxis(FIntPoint& Coord, DeformedAxis AxisToReverse) const
{
	int NumAxisCells = 0;
	int Index = 0;
	ParamsFromAxis(Index, NumAxisCells, AxisToReverse);

	LoopAxis(Coord, AxisToReverse);
	Coord[Index] = NumAxisCells - Coord[Index] - 1;
}

int USphereRule::ApplyRule(FIntPoint Coord) const
{
	int XAxisCrossed = IsAxisTwisted(Coord, DeformedAxis::XAxis);
	int ZAxisCrossed = IsAxisTwisted(Coord, DeformedAxis::ZAxis);

	int RotationTile = XAxisCrossed + 2 * ZAxisCrossed;

	// how to virtually change grid orientation (cell is considered "fixed" as grid rotates around it)
	switch (RotationTile)
	{
	case 0:
		// 0 degrees
		break;
		
	case 1:
		// 90 degrees CW

		
		LoopAxis(Coord, DeformedAxis::XAxis);
		ReverseAxis(Coord, DeformedAxis::ZAxis);

		Coord = FIntPoint(Coord[1], Coord[0]);

		LoopAxis(Coord, DeformedAxis::XAxis);
		LoopAxis(Coord, DeformedAxis::ZAxis);

		break;

		// X axis becomes Z axis
		// Z axis becomes -X axis

	case 2:
		// 270 degrees CW

		
		ReverseAxis(Coord, DeformedAxis::XAxis);
		LoopAxis(Coord, DeformedAxis::ZAxis);

		Coord = FIntPoint(Coord[1], Coord[0]);

		LoopAxis(Coord, DeformedAxis::XAxis);
		LoopAxis(Coord, DeformedAxis::ZAxis);
		break;

		// X axis becomes -Z axis
		// Z axis becomes X axis

	case 3:

		//180 degrees CW

		ReverseAxis(Coord, DeformedAxis::XAxis);
		ReverseAxis(Coord, DeformedAxis::ZAxis);
		break;

		//X axis becomes -X axis
		//Z axis becomes -Z axis
	}

	return CoordToCellID(Coord);
}

int UCrossSurfaceRule::ApplyRule(FIntPoint Coord) const
{

	bool ZAxisTwisted = IsAxisTwisted(Coord, DeformedAxis::ZAxis);
	bool XAxisTwisted = IsAxisTwisted(Coord, DeformedAxis::XAxis);

	if (XAxisTwisted)
	{
		ReverseAxis(Coord, DeformedAxis::ZAxis);
	}
	else
	{
		LoopAxis(Coord, DeformedAxis::ZAxis);
	}

	if (ZAxisTwisted)
	{
		ReverseAxis(Coord, DeformedAxis::XAxis);
	}
	else
	{
		LoopAxis(Coord, DeformedAxis::XAxis);
	}

	return CoordToCellID(Coord);
}

int UKleinRule::ApplyRule(FIntPoint Coord) const
{

	bool ZAxisTwisted = IsAxisTwisted(Coord, DeformedAxis::ZAxis);

	if (ZAxisTwisted)
	{
		ReverseAxis(Coord, DeformedAxis::XAxis);
	}
	else
	{
		LoopAxis(Coord, DeformedAxis::XAxis);
	}

	LoopAxis(Coord, DeformedAxis::ZAxis);

	return CoordToCellID(Coord);

}

int UTorusRule::ApplyRule(FIntPoint Coord) const
{
	LoopAxis(Coord, DeformedAxis::XAxis);
	LoopAxis(Coord, DeformedAxis::ZAxis);

	return CoordToCellID(Coord);
}

int UCylinderRule::ApplyRule(FIntPoint Coord) const
{
	if ((Coord[1] >= 0) && (Coord[1] < NumZCells))
	{
		LoopAxis(Coord, DeformedAxis::XAxis);

		return CoordToCellID(Coord);
	}
	else return -1;
}

int UFiniteRule::ApplyRule(FIntPoint Coord) const
{
	int x = Coord[0];
	int z = Coord[1];

	if (((x >= 0) && (z >= 0)) && ((x < NumXCells) && (z < NumZCells)))
	{
		return CoordToCellID(Coord);
	}
	else return -1;
}

IGridRuleInterface* UBaseGridRuleFactory::CreateGridRuleInterface(BoundGridRuleset Ruleset)
{
	const TSubclassOf<UStandardXZGrid>* RuleTypeResult = RuleEnumToClass.Find(Ruleset);
	TSubclassOf<class UStandardXZGrid> RuleType = RuleTypeResult ? *RuleTypeResult : UStandardXZGrid::StaticClass();

	UObject* CreatedObject = NewObject<UStandardXZGrid>(this->GetOuter(), RuleType);

	IGridRuleInterface* GridRule;
	GridRule = Cast<IGridRuleInterface>(CreatedObject);
	return GridRule;
}

