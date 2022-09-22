#pragma once


//#include "CoreMinimal.h"
#include "Math/IntPoint.h"
#include "HexCoords.generated.h"


//https://www.redblobgames.com/grids/hexagons/


UENUM()
enum class OffsetLayout : uint8
{
	OddR,
	EvenR,
	OddQ,
	EvenQ
};


namespace HexCoords
{
	FIntPoint OffsetToAxial(FIntPoint OffsetCoord, OffsetLayout Layout = OffsetLayout::OddQ);


	FIntPoint OffsetToAxialOddR(FIntPoint OffsetCoord);


	FIntPoint OffsetToAxialEvenR(FIntPoint OffsetCoord);


	FIntPoint OffsetToAxialOddQ(FIntPoint OffsetCoord);


	FIntPoint OffsetToAxialEvenQ(FIntPoint OffsetCoord);




	FIntPoint AxialToOffset(FIntPoint AxialCoord, OffsetLayout Layout = OffsetLayout::OddQ);


	FIntPoint AxialToOffsetOddR(FIntPoint OffsetCoord);


	FIntPoint AxialToOffsetEvenR(FIntPoint OffsetCoord);


	FIntPoint AxialToOffsetOddQ(FIntPoint OffsetCoord);


	FIntPoint AxialToOffsetEvenQ(FIntPoint OffsetCoord);




	FVector2D OffsetToTransform(FIntPoint OffsetCoord, OffsetLayout Layout = OffsetLayout::OddQ);


	FVector2D OffsetToTransformOddR(FIntPoint OffsetCoord);


	FVector2D OffsetToTransformEvenR(FIntPoint OffsetCoord);


	FVector2D OffsetToTransformOddQ(FIntPoint OffsetCoord);


	FVector2D OffsetToTransformEvenQ(FIntPoint OffsetCoord);


}





