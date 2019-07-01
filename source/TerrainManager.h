#pragma once

#include <cmgCore/cmg_core.h>
#include <cmgGraphics/cmg_graphics.h>
#include "CommonTypes.h"
#include "Chunk.h"


struct ChunkInfo
{
	EntityHandle entity;
	Vector3i coord;
	uint32 lodIndex;
	bool valid;
};


class TerrainManager
{
public:
	TerrainManager();
	virtual ~TerrainManager();
	
	void SetFocus(const Vector3f& focus);
	void Clear();
	void UpdateChunks(float timeDelta);
	uint32 CalcLODIndex(const Vector3i& coord);
	float CalcDistToChunk(const Vector3i& coord);

protected:
	virtual EntityHandle CreateChunk(const Vector3f& position,
		const Vector3f& size, const Vector3ui& resolution, uint32 lodIndex) = 0;
	virtual void DeleteChunk(EntityHandle chunk) = 0;

private:
	ChunkInfo GetChunk(const Vector3i& coord, uint32 lodIndex);
	ChunkInfo CreateChunk(const Vector3i& coord, uint32 lodIndex);
	void DeleteChunk(const ChunkInfo& chunk);

protected:
	// Static settings
	Vector3f m_chunkSize;
	float m_radius;
	float m_lodRadius;
	uint32 m_maxLODCount;
	Vector3ui m_chunkResolution;
	Vector3i m_chunkBoundsMin;
	Vector3i m_chunkBoundsMax;
	uint32 m_maxCreatesPerUpdate;

	// Dynamic settings
	Vector3f m_focus;

	// Internal data storage
	std::map<Vector3i, ChunkInfo> m_chunkMap;
};

