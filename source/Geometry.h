#pragma once

#include <cmgCore/cmg_core.h>
#include <cmgGraphics/cmg_graphics.h>
#include "Biarc.h"
#include "RoadCurves.h"


class Geometry
{
public:


	static void ZipArcs(
		Array<VertexPosNorm>& outVertices,
		Array<unsigned int>& outIndices,
		const Array<RoadCurveLine>& left,
		const Array<RoadCurveLine>& right);
	static void ZipArcs(
		Array<VertexPosNorm>& outVertices,
		Array<unsigned int>& outIndices,
		const Array<VertexPosNorm>& left,
		const Array<VertexPosNorm>& right);
};
