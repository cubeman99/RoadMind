#pragma once

#include <cmgGraphics/cmg_graphics.h>
#include <cmgMath/cmg_math.h>
#include <map>
#include <vector>


struct MeshComponent : public ECSComponent<MeshComponent>
{
	Mesh::sptr mesh;
};

