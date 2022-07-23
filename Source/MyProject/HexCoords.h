#pragma once

#include "CoreMinimal.h"
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
	FIntPoint OffsetToAxial(FIntPoint OffsetCoord, OffsetLayout Layout);

	FIntPoint OffsetToAxialOddR(FIntPoint OffsetCoord);

	FIntPoint OffsetToAxialEvenR(FIntPoint OffsetCoord);

	FIntPoint OffsetToAxialOddQ(FIntPoint OffsetCoord);

	FIntPoint OffsetToAxialEvenQ(FIntPoint OffsetCoord);


	FIntPoint AxialToOffset(FIntPoint AxialCoord, OffsetLayout Layout);

	FIntPoint AxialToOffsetOddR(FIntPoint OffsetCoord);

	FIntPoint AxialToOffsetEvenR(FIntPoint OffsetCoord);

	FIntPoint AxialToOffsetOddQ(FIntPoint OffsetCoord);

	FIntPoint AxialToOffsetEvenQ(FIntPoint OffsetCoord);
}



