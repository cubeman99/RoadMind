#include "NodeGroupConnection.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroupConnection::NodeGroupConnection()
	: m_metrics(nullptr)
	, m_twin(nullptr)
{
}

NodeGroupConnection::~NodeGroupConnection()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

const RoadMetrics* NodeGroupConnection::GetMetrics() const
{
	return m_metrics;
}

BiarcPair NodeGroupConnection::GetLeftEdgeLine() const
{
	return m_dividerLines.front();
}

BiarcPair NodeGroupConnection::GetRightEdgeLine() const
{
	return m_dividerLines.back();
}

NodeGroupConnection* NodeGroupConnection::GetTwin()
{
	return m_twin;
}

const NodeSubGroup& NodeGroupConnection::GetInput()
{
	return m_input;
}

const NodeSubGroup& NodeGroupConnection::GetOutput()
{
	return m_output;
}


//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------

void NodeGroupConnection::UpdateGeometry()
{
	// Create the left edge
	Node* nodes[2];
	nodes[0] = m_input.group->GetNode(m_input.index);
	nodes[1] = m_output.group->GetNode(m_output.index);
	BiarcPair prev, curr;
	prev = BiarcPair::Interpolate(
		nodes[0]->m_position, m_input.group->GetDirection(),
		nodes[1]->m_position, m_output.group->GetDirection());
	m_dividerLines.clear();
	m_dividerLines.push_back(prev);

	// Create the lane dividers
	int minRightCount = Math::Min(m_input.count, m_output.count);
	int mxaRightCount = Math::Max(m_input.count, m_output.count);
	for (int i = 0; i < minRightCount; i++)
	{
		nodes[0] = m_input.group->GetNode(i);
		nodes[1] = m_output.group->GetNode(i);
		curr = BiarcPair::CreateParallel(prev,
			nodes[0]->GetWidth(), nodes[1]->GetWidth());
		m_dividerLines.push_back(curr);
		prev = curr;
	}

	// Create the right edge
	float offsets[2] = { 0.0f, 0.0f };
	for (int k = 0; k < 2; k++)
	{
		for (int i = minRightCount; i < m_groups[k].count; i++)
			offsets[k] += m_groups[k].group->GetNode(i)->GetWidth();
	}
	curr = BiarcPair::CreateParallel(prev, offsets[0], offsets[1]);
	m_dividerLines.push_back(curr);
}

