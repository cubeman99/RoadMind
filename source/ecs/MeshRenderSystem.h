#pragma once

#include <cmgGraphics/cmg_graphics.h>
#include <cmgMath/cmg_math.h>
#include <map>
#include <vector>
#include "ecs/MeshComponent.h"
#include "ecs/MaterialComponent.h"
#include "Camera.h"


class MeshRenderSystem : public BaseECSSystem
{
public:
	MeshRenderSystem(DebugDraw& debugDraw, RenderDevice& renderDevice);

	void SetCamera(Camera* camera);

	virtual void UpdateComponents(float delta, BaseECSComponent** components);

private:
	DebugDraw& m_debugDraw;
	RenderDevice& m_renderDevice;
	Camera* m_camera;
};
