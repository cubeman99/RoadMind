#include "NodeGroupConnection.h"
#include "NodeGroupTie.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroupConnection::NodeGroupConnection()
	: m_metrics(nullptr)
	, m_laneIntersectionPoint(Vector2f::ZERO)
{
}

NodeGroupConnection::~NodeGroupConnection()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

int NodeGroupConnection::GetId() const
{
	return m_id;
}

const RoadMetrics* NodeGroupConnection::GetMetrics() const
{
	return m_metrics;
}

BiarcPair NodeGroupConnection::GetLeftEdgeLine() const
{
	return m_edgeLines[(int) LaneSide::LEFT];
}

BiarcPair NodeGroupConnection::GetRightEdgeLine() const
{
	return m_edgeLines[(int) LaneSide::RIGHT];
}

RoadCurveLine NodeGroupConnection::GetLeftVisualEdgeLine() const
{
	return m_visualEdgeLines[(int) LaneSide::LEFT];
}

RoadCurveLine NodeGroupConnection::GetRightVisualEdgeLine() const
{
	return m_visualEdgeLines[(int) LaneSide::RIGHT];
}

RoadCurveLine NodeGroupConnection::GetLeftVisualShoulderLine() const
{
	return m_visualShoulderLines[(int) LaneSide::LEFT];
}

RoadCurveLine NodeGroupConnection::GetRightVisualShoulderLine() const
{
	return m_visualShoulderLines[(int) LaneSide::RIGHT];
}

NodeGroupConnection* NodeGroupConnection::GetTwin()
{
	NodeGroup* inputTwin = GetInput().group->m_twin;
	NodeGroup* outputTwin = GetOutput().group->m_twin;

	if (GetInput().index == 0 && GetOutput().index == 0 &&
		inputTwin != nullptr && outputTwin != nullptr &&
		!GetInput().group->m_tie->IsDivided() &&
		!GetOutput().group->m_tie->IsDivided())
	{
		for (unsigned int i = 0; i < outputTwin->GetOutputs().size() &&
			outputTwin->GetOutputs()[i]->GetInput().index == 0; i++)
		{
			NodeGroupConnection* connection = outputTwin->GetOutputs()[i];
			if (connection->GetOutput().index == 0 &&
				connection->GetOutput().group == inputTwin)
				return connection;
		}
	}

	return nullptr;
}

NodeSubGroup& NodeGroupConnection::GetInput()
{
	return m_groups[(int) InputOutput::INPUT];
}

NodeSubGroup& NodeGroupConnection::GetOutput()
{
	return m_groups[(int) InputOutput::OUTPUT];
}

void NodeGroupConnection::SetInput(const NodeSubGroup& input)
{
	m_groups[(int) InputOutput::INPUT] = input;
}

void NodeGroupConnection::SetOutput(const NodeSubGroup& output)
{
	m_groups[(int) InputOutput::OUTPUT] = output;
}

const Array<BiarcPair>& NodeGroupConnection::GetSeams(IOType type, LaneSide side) const
{
	return m_seams[(int) type][(int) side];
}

Array<BiarcPair>& NodeGroupConnection::GetSeams(IOType type, LaneSide side)
{
	return m_seams[(int) type][(int) side];
}


//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------

void NodeGroupConnection::SetSeam(IOType end, LaneSide side, const BiarcPair& seam)
{
	Array<BiarcPair>& seams = GetSeams(end, side);
	seams.resize(1);
	seams[0] = seam;
}

void NodeGroupConnection::AddSeam(IOType end, LaneSide side, const BiarcPair& seam)
{
	Array<BiarcPair>& seams = GetSeams(end, side);
	seams.push_back(seam);
}

void NodeGroupConnection::UpdateGeometry()
{
	// Create the left edge
	Node* nodes[2];
	nodes[0] = GetInput().group->GetNode(GetInput().index);
	nodes[1] = GetOutput().group->GetNode(GetOutput().index);
	BiarcPair prev, curr;
	prev = BiarcPair::Interpolate(
		nodes[0]->m_position.xy, GetInput().group->GetDirection(),
		nodes[1]->m_position.xy, GetOutput().group->GetDirection());
	m_dividerLines.clear();
	m_dividerLines.push_back(prev);

	// Create the lane dividers
	int minRightCount = Math::Min(GetInput().count, GetOutput().count);
	int mxaRightCount = Math::Max(GetInput().count, GetOutput().count);
	for (int i = 0; i < minRightCount; i++)
	{
		nodes[0] = GetInput().group->GetNode(i);
		nodes[1] = GetOutput().group->GetNode(i);
		curr = BiarcPair::CreateParallel(prev,
			nodes[0]->GetWidth(), nodes[1]->GetWidth());
		m_dividerLines.push_back(curr);
		prev = curr;
	}

	// Create the right edge
	if (GetInput().count != GetOutput().count)
	{
		float offsets[2] = { 0.0f, 0.0f };
		for (int k = 0; k < 2; k++)
		{
			for (int i = minRightCount; i < m_groups[k].count; i++)
				offsets[k] += m_groups[k].group->GetNode(i)->GetWidth();
		}
		curr = BiarcPair::CreateParallel(prev, offsets[0], offsets[1]);
		m_dividerLines.push_back(curr);
	}

	m_edgeLines[(int) LaneSide::LEFT] =
		BiarcPair::CreateParallel(m_dividerLines.front(),
		-GetInput().group->GetLeftShoulderWidth(),
		-GetOutput().group->GetLeftShoulderWidth());
	m_edgeLines[(int) LaneSide::RIGHT] =
		BiarcPair::CreateParallel(m_dividerLines.back(),
		GetInput().group->GetRightShoulderWidth(),
		GetOutput().group->GetRightShoulderWidth());

	VerticalCurve verticalCurve(GetInput().group->GetPosition().z,
		GetOutput().group->GetPosition().z);
	m_visualEdgeLines[0] = RoadCurveLine(m_dividerLines.front(), verticalCurve);
	m_visualEdgeLines[1] = RoadCurveLine(m_dividerLines.back(), verticalCurve);
	m_visualShoulderLines[0] = RoadCurveLine(m_edgeLines[0], verticalCurve);
	m_visualShoulderLines[1] = RoadCurveLine(m_edgeLines[1], verticalCurve);

	m_seams[0][0].clear();
	m_seams[0][1].clear();
	m_seams[1][0].clear();
	m_seams[1][1].clear();
}

