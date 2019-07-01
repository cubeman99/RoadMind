#include "TerrainManager.h"


TerrainManager::TerrainManager():
	m_chunkResolution(32),
	m_chunkSize(120.0f),
	m_focus(Vector3f::ZERO),
	//m_maxLODCount(5),
	m_maxLODCount(5),
	m_maxCreatesPerUpdate(8)
{
	m_radius = m_chunkSize.x * 4;
	m_lodRadius = m_chunkSize.x * 1.0f;

	int32 r = 10000;
	m_chunkBoundsMin = Vector3i(-r, -r, 0);
	m_chunkBoundsMax = Vector3i(r + 1, r + 1, 1);
}

TerrainManager::~TerrainManager()
{
}

void TerrainManager::SetFocus(const Vector3f& focus)
{
	m_focus = focus;
}

void TerrainManager::Clear()
{
	Array<ChunkInfo> chunks;
	for (auto it = m_chunkMap.begin(); it != m_chunkMap.end(); it++)
		chunks.push_back(it->second);
	for (uint32 i = 0; i < chunks.size(); i++)
	{
		DeleteChunk(chunks[i]);
	}
	m_chunkMap.clear();
}

void TerrainManager::UpdateChunks(float timeDelta)
{
	Vector3i mins, maxs, coord;
	for (uint32 i = 0; i < 3; i++)
	{
		mins[i] = (int) Math::Floor((m_focus[i] - m_radius) / m_chunkSize[i]);
		maxs[i] = (int) Math::Ceil((m_focus[i] + m_radius) / m_chunkSize[i]);
		mins[i] = Math::Max(mins[i], m_chunkBoundsMin[i]);
		maxs[i] = Math::Min(maxs[i], m_chunkBoundsMax[i]);
	}

	uint32 createCount = 0;

	// Remove chunks too far away
	Array<ChunkInfo> chunks;
	for (auto it = m_chunkMap.begin(); it != m_chunkMap.end(); it++)
		chunks.push_back(it->second);
	for (uint32 i = 0; i < chunks.size(); i++)
	{
		ChunkInfo chunk = chunks[i];
		float dist = CalcDistToChunk(chunk.coord);
		uint32 lodIndex = CalcLODIndex(chunk.coord);
		if (dist >= m_radius)
		{
			DeleteChunk(chunk);
		}
		else if (chunk.lodIndex != lodIndex && createCount < m_maxCreatesPerUpdate)
		{
			createCount += 1;
			DeleteChunk(chunk);
			CreateChunk(coord, lodIndex);
		}
	}

	// Create near chunks
	coord = Vector3i::ZERO;
	for (coord.z = mins.z; coord.z < maxs.z; coord.z++)
	{
		for (coord.y = mins.y; coord.y < maxs.y; coord.y++)
		{
			for (coord.x = mins.x; coord.x < maxs.x; coord.x++)
			{
				float dist = CalcDistToChunk(coord);
				uint32 lodIndex = CalcLODIndex(coord);
				if (dist < m_radius && !GetChunk(coord, 0).valid &&
					createCount < m_maxCreatesPerUpdate)
				{
					CreateChunk(coord, lodIndex);
					createCount += 1;
				}
			}
		}
	}
}

uint32 TerrainManager::CalcLODIndex(const Vector3i & coord)
{
	float dist = CalcDistToChunk(coord);
	uint32 lodIndex = (uint32) (dist / m_lodRadius);
	while (m_chunkResolution.x / (1 << lodIndex) < 2)
		lodIndex--;
	return lodIndex;
}

float TerrainManager::CalcDistToChunk(const Vector3i & coord)
{
	Bounds bounds;
	bounds.mins = Vector3f(coord) * m_chunkSize;
	bounds.maxs = bounds.mins + m_chunkSize;
	return bounds.DistToPoint(m_focus);
}

ChunkInfo TerrainManager::GetChunk(const Vector3i & coord, uint32 lodIndex)
{
	if (m_chunkMap.find(coord) != m_chunkMap.end())
		return m_chunkMap[coord];
	ChunkInfo chunk;
	chunk.coord = coord;
	chunk.valid = false;
	return chunk;
}

ChunkInfo TerrainManager::CreateChunk(const Vector3i & coord, uint32 lodIndex)
{
	ChunkInfo chunk;
	chunk.coord = coord;
	chunk.lodIndex = lodIndex;
	chunk.valid = true;
	Vector3f position = Vector3f(coord) * m_chunkSize;
	Vector3ui resolution = m_chunkResolution / (1 << lodIndex);
	chunk.entity = CreateChunk(position, m_chunkSize, resolution, lodIndex);
	m_chunkMap[coord] = chunk;
	return chunk;
}

void TerrainManager::DeleteChunk(const ChunkInfo& chunk)
{
	DeleteChunk(chunk.entity);
	m_chunkMap.erase(chunk.coord);
}
