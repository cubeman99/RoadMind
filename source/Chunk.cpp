#include "Chunk.h"


Chunk::Chunk(const Vector3i& coord, uint32 lodIndex) :
	m_coord(coord),
	m_lodIndex(lodIndex)
{
}

