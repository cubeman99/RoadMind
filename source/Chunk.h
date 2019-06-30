#pragma once

#include <cmgGraphics/cmg_graphics.h>
#include <cmgMath/cmg_math.h>
#include <map>
#include <vector>
#include "ecs/MeshComponent.h"
#include "ecs/MaterialComponent.h"
#include "Camera.h"


class Chunk : public ECSComponent<Chunk>
{
public:
	Chunk(const Vector3i& coord, uint32 lodIndex = 0);

	inline uint32 GetLODIndex() const { return m_lodIndex; }

private:
	Vector3i m_coord;
	uint32 m_lodIndex;
};
