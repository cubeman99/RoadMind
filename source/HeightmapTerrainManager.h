#pragma once

#include <cmgCore/cmg_core.h>
#include <cmgGraphics/cmg_graphics.h>
#include "CommonTypes.h"
#include "Chunk.h"
#include "TerrainManager.h"


class HeightmapTerrainManager : public TerrainManager, public BaseECSSystem
{
public:
	HeightmapTerrainManager(RenderDevice* renderDevice,
		const MaterialComponent& material, ECS& ecs);
	virtual ~HeightmapTerrainManager();

	void SetGenerateVerticesShader(Shader* shader);
	void SetGenerateNormalsShader(Shader* shader);
	virtual void UpdateComponents(float delta, BaseECSComponent** components) override;

protected:
	virtual EntityHandle CreateChunk(const Vector3f& position,
		const Vector3f& size, const Vector3ui& resolution, uint32 lodIndex);
	virtual void DeleteChunk(EntityHandle chunk);
	//void SewSeam(const Vector3i& coord, const Vector3i& neighborCoord);

public:
	Shader* m_shaderGenerateVertices;
	Shader* m_shaderGenerateNormals;

private:
	uint32 GetVertexIndex(Vector2i point, uint32 lodIndex) const;

	ECS& m_ecs;
	RenderDevice* m_renderDevice;
	MaterialComponent m_terrainMaterial;

	ShaderStorageBuffer m_bufferVertices;
	ShaderStorageBuffer m_bufferIndices[8];
};

