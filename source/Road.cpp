#include "Road.h"

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

RoadSurface::RoadSurface()
	: m_metrics(nullptr)
	, m_twin(nullptr)
{
}

RoadSurface::~RoadSurface()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

const RoadMetrics* RoadSurface::GetMetrics() const
{
	return m_metrics;
}

BiarcPair RoadSurface::GetLeftEdgeLine() const
{
	return m_dividerLines.front();
}

BiarcPair RoadSurface::GetRightEdgeLine() const
{
	return m_dividerLines.back();
}

RoadSurface* RoadSurface::GetTwin()
{
	return m_twin;
}

NodeGroup* RoadSurface::GetInput()
{
	return m_groups[0];
}

NodeGroup* RoadSurface::GetOutput()
{
	return m_groups[1];
}


//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------

void RoadSurface::UpdateGeometry()
{
	// Create the left edge
	BiarcPair prev, curr;
	prev = BiarcPair::Interpolate(
		m_groups[0]->GetPosition(), m_groups[0]->GetDirection(),
		m_groups[1]->GetPosition(), m_groups[1]->GetDirection());
	m_dividerLines.clear();
	m_dividerLines.push_back(prev);

	// Create the lane dividers
	Node* nodes[2];
	nodes[0] = m_groups[0]->GetLeftNode();
	nodes[1] = m_groups[1]->GetLeftNode();
	int minRightCount = Math::Min(m_counts[0], m_counts[1]);
	int mxaRightCount = Math::Max(m_counts[0], m_counts[1]);
	for (int i = 0; i < minRightCount; i++)
	{
		curr = BiarcPair::CreateParallel(prev,
			nodes[0]->GetWidth(), nodes[1]->GetWidth());
		m_dividerLines.push_back(curr);
		prev = curr;
		nodes[0] = nodes[0]->m_rightNode;
		nodes[1] = nodes[1]->m_rightNode;
	}

	// Create the right edge
	float offsets[2] = { 0.0f, 0.0f };
	for (int k = 0; k < 2; k++)
	{
		for (int i = minRightCount; i < m_counts[k]; i++)
		{
			offsets[k] += nodes[k]->GetWidth();
			nodes[k] = nodes[k]->GetRightNode();
		}
	}
	curr = BiarcPair::CreateParallel(prev, offsets[0], offsets[1]);
	m_dividerLines.push_back(curr);
}

