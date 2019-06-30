#pragma once

#include <cmgCore/cmg_core.h>
#include <cmgGraphics/cmg_graphics.h>
#include "CommonTypes.h"
#include "Chunk.h"


template <typename T>
struct Vector3Compare
{
	bool operator() (const Vector3<T>& left, const Vector3<T>& right) const
	{
		if (left.x < right.x)
			return true;
		else if (left.x == right.x)
		{
			if (left.y < right.y)
				return true;
			else if (left.y == right.y)
				return (left.z < right.z);
		}
		return false;
	}
};

class MarchingCubes : public BaseECSSystem
{
public:
	MarchingCubes(RenderDevice* renderDevice, Shader* shaderMarchingCubes, Shader* shaderNoise,
		const MaterialComponent& material, ECS& ecs);
	~MarchingCubes();

	void SetFocus(const Vector3f& focus);
	void SetMarchingCubesShader(Shader* shader);
	void SetDensityShader(Shader* shader);
	void SetHeightmapShader(Shader* shader);
	virtual void PreUpdate(float timeDelta) override;
	virtual void UpdateComponents(float delta, BaseECSComponent** components) override;

	void RecreateChunks();
	uint32 CalcLODIndex(const Vector3i& coord);
	float CalcDistToChunk(const Vector3i& coord);
	EntityHandle CreateChunk(const Vector3i& coord);
	void RemoveChunk(const Vector3i& coord);
	Mesh* CreateMesh(const Vector3f& offset, uint32 lodIndex);
	Chunk* GetChunk(const Vector3i& coord);
	void SewSeam(const Vector3i& coord, const Vector3i& neighborCoord);

public:
	Shader* m_shaderGenerateVertices;
	Shader* m_shaderGenerateNormals;

private:
	uint32 GetVertexIndex(Vector2i point, uint32 lodIndex) const;

	ECS& m_ecs;
	RenderDevice* m_renderDevice;
	Shader* m_shaderHeightmap;
	Shader* m_shaderMarchingCubes;
	Shader* m_shaderNoise;
	MaterialComponent m_terrainMaterial;
	Vector3f m_focus;

	Array<EntityHandle> m_chunks;
	Vector3f m_chunkSize;
	float m_radius;
	float m_lodRadius;
	uint32 m_maxLODCount;
	std::map<Vector3i, EntityHandle, Vector3Compare<int32>> m_chunkMap;
	Vector3ui m_chunkResolution;

	ShaderStorageBuffer m_bufferVertices;
	ShaderStorageBuffer m_bufferIndices[8];
	ShaderStorageBuffer m_bufferPoints;
	ShaderStorageBuffer m_bufferLookup;
	AtomicCounterBuffer m_bufferAtomicCounter;
};

