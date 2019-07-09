#pragma once

#include <cmgCore/cmg_core.h>
#include <cmgGraphics/cmg_graphics.h>
#include "CommonTypes.h"
#include "Chunk.h"
#include "TerrainManager.h"


class DensityTerrainManager : public TerrainManager, public BaseECSSystem
{
public:
	DensityTerrainManager(RenderDevice* renderDevice,
		const MaterialComponent& material, ECS& ecs);
	virtual ~DensityTerrainManager();

	void SetMarchingCubesShader(Shader* shader);
	void SetDensityShader(Shader* shader);
	virtual void UpdateComponents(float delta, BaseECSComponent** components) override;

protected:
	virtual EntityHandle CreateChunk(const Vector3f& position,
		const Vector3f& size, const Vector3ui& resolution, uint32 lodIndex);
	virtual void DeleteChunk(EntityHandle chunk);

private:
public:
	ECS& m_ecs;
	RenderDevice* m_renderDevice;
	Shader* m_shaderGenerateDensity;
	Shader* m_shaderListNonEmptyCells;
	Shader* m_shaderListVertices;
	Shader* m_shaderMarchingCubes;
	Shader* m_shaderGenerateVertices;
	Shader* m_shaderGenerateIndices;
	MaterialComponent m_terrainMaterial;

	Texture* m_vertexIdTexture;

	ShaderStorageBuffer m_bufferNonEmptyCells;
	ShaderStorageBuffer m_bufferVertexEdges;
	ShaderStorageBuffer m_bufferVertices;
	ShaderStorageBuffer m_bufferIndices;
	ShaderStorageBuffer m_bufferPoints;
	ShaderStorageBuffer m_bufferLookup;
	AtomicCounterBuffer m_bufferAtomicCounter;
	ShaderStorageBuffer m_bufferVertexIdLookup;
};

