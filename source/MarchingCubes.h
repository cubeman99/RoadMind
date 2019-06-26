#pragma once

#include <cmgGraphics/cmg_graphics.h>
#include "CommonTypes.h"


class MarchingCubes
{
public:

	static void CreateMesh(RenderDevice* renderDevice, Shader* shader, Shader* shaderTerrain,
		Mesh* outMesh, uint32 width, uint32 height, uint32 depth);
};

