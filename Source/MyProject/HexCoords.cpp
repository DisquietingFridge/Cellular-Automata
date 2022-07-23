#include "HexCoords.h"
#include "CoreMinimal.h"

using namespace HexCoords;


FIntPoint HexCoords::OffsetToAxial(FIntPoint OffsetCoord, OffsetLayout Layout = OffsetLayout::OddR)
{
	switch (Layout)
	{
	case OffsetLayout::OddR:
		return OffsetToAxialOddR(OffsetCoord);
		break;

	case OffsetLayout::EvenR:
		return OffsetToAxialEvenR(OffsetCoord);
		break;
	case OffsetLayout::OddQ:
		return OffsetToAxialOddQ(OffsetCoord);
		break;
	case OffsetLayout::EvenQ:
		return OffsetToAxialEvenQ(OffsetCoord);
		break;
	}
	return FIntPoint(0);
}

FIntPoint HexCoords::OffsetToAxialOddR(FIntPoint OffsetCoord)
{
	int q = OffsetCoord[0] - (OffsetCoord[1] - (OffsetCoord[1]&1)) / 2;
	int r = OffsetCoord[1];

	return FIntPoint(q, r);
}

FIntPoint HexCoords::OffsetToAxialEvenR(FIntPoint OffsetCoord)
{
	int q = OffsetCoord[0] - (OffsetCoord[1] + (OffsetCoord[1] & 1)) / 2;
	int r = OffsetCoord[1];

	return FIntPoint(q, r);
}

FIntPoint HexCoords::OffsetToAxialOddQ(FIntPoint OffsetCoord)
{
	int q = OffsetCoord[0];
	int r = OffsetCoord[1] - (OffsetCoord[0] - (OffsetCoord[0] & 1)) / 2;

	return FIntPoint(q, r);
}

FIntPoint HexCoords::OffsetToAxialEvenQ(FIntPoint OffsetCoord)
{
	int q = OffsetCoord[0];
	int r = OffsetCoord[1] - (OffsetCoord[0] + (OffsetCoord[0] & 1)) / 2;

	return FIntPoint(q, r);
}

FIntPoint HexCoords::AxialToOffset(FIntPoint AxialCoord, OffsetLayout Layout = OffsetLayout::OddR)
{
	switch (Layout)
	{
	case OffsetLayout::OddR:
		return AxialToOffsetOddR(AxialCoord);
		break;

	case OffsetLayout::EvenR:
		return AxialToOffsetEvenR(AxialCoord);
		break;
	case OffsetLayout::OddQ:
		return AxialToOffsetOddQ(AxialCoord);
		break;
	case OffsetLayout::EvenQ:
		return AxialToOffsetEvenQ(AxialCoord);
		break;
	}
	return FIntPoint(0);
}

FIntPoint HexCoords::AxialToOffsetOddR(FIntPoint AxialCoord)
{
	int col = AxialCoord[0] + (AxialCoord[1] - (AxialCoord[1] & 1)) / 2;
	int row = AxialCoord[1];

	return FIntPoint(col, row);
}

FIntPoint HexCoords::AxialToOffsetEvenR(FIntPoint AxialCoord)
{
	int col = AxialCoord[0] + (AxialCoord[1] + (AxialCoord[1] & 1)) / 2;
	int row = AxialCoord[1];

	return FIntPoint(col, row);
}

FIntPoint HexCoords::AxialToOffsetOddQ(FIntPoint AxialCoord)
{
	int col = AxialCoord[0] ;
	int row = AxialCoord[1] + (AxialCoord[0] - (AxialCoord[0] & 1)) / 2;

	return FIntPoint(col, row);
}

FIntPoint HexCoords::AxialToOffsetEvenQ(FIntPoint AxialCoord)
{
	int col = AxialCoord[0];
	int row = AxialCoord[1] + (AxialCoord[0] + (AxialCoord[0] & 1)) / 2;

	return FIntPoint(col, row);
}