#include "HexCoords.h"
#include "CoreMinimal.h"


using namespace HexCoords;

FIntPoint HexCoords::OffsetToAxial(FIntPoint OffsetCoord, OffsetLayout Layout)
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
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	int q = XCoord - (ZCoord - (ZCoord & 1)) / 2;
	int r = ZCoord;


	return FIntPoint(q, r);
}


FIntPoint HexCoords::OffsetToAxialEvenR(FIntPoint OffsetCoord)
{
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	int q = OffsetCoord[0] - (OffsetCoord[1] + (OffsetCoord[1] & 1)) / 2;
	int r = OffsetCoord[1];


	return FIntPoint(q, r);
}


FIntPoint HexCoords::OffsetToAxialOddQ(FIntPoint OffsetCoord)
{
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	int q = OffsetCoord[0];
	int r = OffsetCoord[1] - (OffsetCoord[0] - (OffsetCoord[0] & 1)) / 2;


	return FIntPoint(q, r);
}


FIntPoint HexCoords::OffsetToAxialEvenQ(FIntPoint OffsetCoord)
{
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	int q = OffsetCoord[0];
	int r = OffsetCoord[1] - (OffsetCoord[0] + (OffsetCoord[0] & 1)) / 2;


	return FIntPoint(q, r);
}


FIntPoint HexCoords::AxialToOffset(FIntPoint AxialCoord, OffsetLayout Layout)
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
	int XCoord = AxialCoord[0] + (AxialCoord[1] - (AxialCoord[1] & 1)) / 2;
	int ZCoord = AxialCoord[1];


	return FIntPoint(XCoord, ZCoord);
}


FIntPoint HexCoords::AxialToOffsetEvenR(FIntPoint AxialCoord)
{
	int XCoord = AxialCoord[0] + (AxialCoord[1] + (AxialCoord[1] & 1)) / 2;
	int ZCoord = AxialCoord[1];


	return FIntPoint(XCoord, ZCoord);
}


FIntPoint HexCoords::AxialToOffsetOddQ(FIntPoint AxialCoord)
{
	int XCoord = AxialCoord[0];
	int ZCoord = AxialCoord[1] + (AxialCoord[0] - (AxialCoord[0] & 1)) / 2;


	return FIntPoint(XCoord, ZCoord);
}


FIntPoint HexCoords::AxialToOffsetEvenQ(FIntPoint AxialCoord)
{
	int XCoord = AxialCoord[0];
	int ZCoord = AxialCoord[1] + (AxialCoord[0] + (AxialCoord[0] & 1)) / 2;


	return FIntPoint(XCoord, ZCoord);
}


FVector2D HexCoords::OffsetToTransform(FIntPoint OffsetCoord, OffsetLayout Layout)
{
	switch (Layout)
	{
	case OffsetLayout::OddR:
		return OffsetToTransformOddR(OffsetCoord);
		break;


	case OffsetLayout::EvenR:
		return OffsetToTransformEvenR(OffsetCoord);
		break;
	case OffsetLayout::OddQ:
		return OffsetToTransformOddQ(OffsetCoord);
		break;
	case OffsetLayout::EvenQ:
		return OffsetToTransformEvenQ(OffsetCoord);
		break;
	}
	return FIntPoint(0);
}


FVector2D HexCoords::OffsetToTransformOddR(FIntPoint OffsetCoord)
{
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	float XTransform = (XCoord + 0.5 * (ZCoord & 1)) * sqrt(3) / 2;


	float ZTransform = ZCoord * 3.0f / 4.0f;


	return FVector2D(XTransform, ZTransform);
}


FVector2D HexCoords::OffsetToTransformEvenR(FIntPoint OffsetCoord)
{
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	float XTransform = (XCoord - 0.5 * (ZCoord & 1)) * sqrt(3) / 2;


	float ZTransform = ZCoord * 3.0f / 4.0f;


	return FVector2D(XTransform, ZTransform);
}


FVector2D HexCoords::OffsetToTransformOddQ(FIntPoint OffsetCoord)
{
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	float XTransform = XCoord * 3.0f / 4.0f;

	float ZTransform = (ZCoord - 0.5 * (XCoord & 1)) * sqrt(3) / 2;


	return FVector2D(XTransform, ZTransform);
}


FVector2D HexCoords::OffsetToTransformEvenQ(FIntPoint OffsetCoord)
{
	int XCoord = OffsetCoord[0];
	int ZCoord = OffsetCoord[1];


	float XTransform = XCoord * 3.0f / 4.0f;


	float ZTransform = (ZCoord + 0.5 * (XCoord & 1)) * sqrt(3) / 2;


	return FVector2D(XTransform, ZTransform);
}