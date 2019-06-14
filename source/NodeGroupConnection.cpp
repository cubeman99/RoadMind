#include "NodeGroupConnection.h"
#include "NodeGroupTie.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroupConnection::NodeGroupConnection()
	: m_metrics(nullptr)
	, m_laneIntersectionPoint(Vector2f::ZERO)
	, m_isGhost(false)
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

NodeSubGroup& NodeGroupConnection::GetSubGroup(IOType type)
{
	return m_groups[(int) type];
}

NodeSubGroup& NodeGroupConnection::GetInput()
{
	return m_groups[(int) InputOutput::INPUT];
}

NodeSubGroup& NodeGroupConnection::GetOutput()
{
	return m_groups[(int) InputOutput::OUTPUT];
}


const Array<RoadCurveLine>& NodeGroupConnection::GetSeams(IOType type, LaneSide side) const
{
	return m_seams[(int) type][(int) side];
}

Array<RoadCurveLine>& NodeGroupConnection::GetSeams(IOType type, LaneSide side)
{
	return m_seams[(int) type][(int) side];
}

Array<BiarcPair>& NodeGroupConnection::GetDrivingLines()
{
	return m_drivingLines;
}

BiarcPair NodeGroupConnection::GetDrivingLine(int fromLaneIndex, int toLaneIndex)
{
	// Create the left edge
	Node* nodes[2];
	nodes[0] = GetInput().group->GetNode(GetInput().index);
	nodes[1] = GetOutput().group->GetNode(GetOutput().index);
	BiarcPair leftEdge = BiarcPair::Interpolate(
		nodes[0]->m_position.xy, GetInput().group->GetDirection(),
		nodes[1]->m_position.xy, GetOutput().group->GetDirection());

	// Create the driving lane
	Meters offsets[2] = {0, 0};
	Meters widths[2] = {0, 0};
	for (int i = 0; i <= fromLaneIndex; i++)
	{
		widths[0] = GetInput().group->GetNode(i)->GetWidth();
		offsets[0] += widths[0];
	}
	for (int i = 0; i <= toLaneIndex; i++)
	{
		widths[1] = GetOutput().group->GetNode(i)->GetWidth();
		offsets[1] += widths[1];
	}
	return BiarcPair::CreateParallel(leftEdge,
		offsets[0] - (widths[0] * 0.5f),
		offsets[1] - (widths[1] * 0.5f));
}

BiarcPair NodeGroupConnection::GetDrivingLine(int laneIndex)
{
	return GetDrivingLine(laneIndex, laneIndex);
}

void NodeGroupConnection::GetLaneOutputRange(
	int fromLaneIndex, int& outToLaneIndex, int& outToLaneCount)
{
	int side1 = m_groups[1].count >= m_groups[0].count ? 0 : 1;
	int side2 = 1 - side1;
	int count1 = m_groups[side1].count;
	int count2 = m_groups[side2].count;

	if (m_groups[1].count <= m_groups[0].count)
	{
		int index = 0;
		for (int i = 0; i < m_groups[1].count; i++)
		{
			if (index + m_laneSplit[i] > fromLaneIndex)
			{
				outToLaneIndex = i;
				outToLaneCount = 1;
				return;
			}
			index += m_laneSplit[i];
		}
	}
	else
	{
		int index = 0;
		for (int i = 0; i < m_groups[0].count; i++)
		{
			if (i == fromLaneIndex)
			{
				outToLaneIndex = index;
				outToLaneCount = m_laneSplit[i];
				return;
			}
			index += m_laneSplit[i];
		}
	}
}

bool NodeGroupConnection::IsGhost() const
{
	return m_isGhost;
}

float NodeGroupConnection::GetLinearSlope() const
{
	Vector3f center0 = m_groups[0].GetCenterPosition();
	Vector3f center1 = m_groups[1].GetCenterPosition();
	Meters dy = center1.z - center0.z;
	Meters dx = (m_visualEdgeLines[0].Length() + m_visualEdgeLines[1].Length()) * 0.5f;
	if (dx < FLT_EPSILON)
		return 0.0f;
	else
		return dy / dx;
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void NodeGroupConnection::SetInput(const NodeSubGroup& input)
{
	m_groups[(int) InputOutput::INPUT] = input;
}

void NodeGroupConnection::SetOutput(const NodeSubGroup& output)
{
	m_groups[(int) InputOutput::OUTPUT] = output;
}

void NodeGroupConnection::SetGhost(bool ghost)
{
	m_isGhost = ghost;
}

void NodeGroupConnection::CycleLaneSplit()
{
	int side1 = m_groups[1].count >= m_groups[0].count ? 0 : 1;
	int side2 = 1 - side1;
	int count1 = m_groups[side1].count;
	int count2 = m_groups[side2].count;

	ConstrainLaneSplit();

	m_laneSplit.push_back(m_laneSplit[0]);
	m_laneSplit.erase(m_laneSplit.begin());
}

void NodeGroupConnection::ConstrainLaneSplit()
{
	int side1 = m_groups[1].count >= m_groups[0].count ? 0 : 1;
	int side2 = 1 - side1;
	int count1 = m_groups[side1].count;
	int count2 = m_groups[side2].count;

	m_laneSplit.resize(count1);
	int remaining = count2;
	for (int i = 0; i < count1 - 1; i++)
	{
		m_laneSplit[i] = Math::Clamp(m_laneSplit[i], 1, remaining - (count1 - 1 - i));
		remaining -= m_laneSplit[i];
	}
	m_laneSplit[count1 - 1] = remaining;
}


//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------

void NodeGroupConnection::SetSeam(IOType end, LaneSide side, const RoadCurveLine& seam)
{
	Array<RoadCurveLine>& seams = GetSeams(end, side);
	seams.resize(1);
	seams[0] = seam;
}

void NodeGroupConnection::AddSeam(IOType end, LaneSide side, const RoadCurveLine& seam)
{
	Array<RoadCurveLine>& seams = GetSeams(end, side);
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

	int side1 = m_groups[1].count >= m_groups[0].count ? 0 : 1;
	int side2 = 1 - side1;
	int count1 = m_groups[side1].count;
	int count2 = m_groups[side2].count;
	NodeSubGroup& group1 = m_groups[side1];
	NodeSubGroup& group2 = m_groups[side2];

	float scale = 1.0f;
	if (side1 == 1)
	{
		m_dividerLines[0] = m_dividerLines[0].Reverse();
		scale = -1.0f;
	}

	// Create the lane dividers
	int j = 0;
	ConstrainLaneSplit();
	for (int i = 0; i < count1; i++)
	{
		Meters w0 = group1.group->GetNode(i)->GetWidth();
		Meters w1 = 0.0f;
		for (int k = 0; k < m_laneSplit[i]; k++)
			w1 += group2.GetNode(j++)->GetWidth();
		m_dividerLines.push_back(BiarcPair::CreateParallel(
			m_dividerLines[i], w0 * scale, w1 * scale));
	}
	if (side1 == 1)
	{
		for (unsigned int i = 0; i < m_dividerLines.size(); i++)
			m_dividerLines[i] = m_dividerLines[i].Reverse();
	}

	// Create the left and right edges
	m_edgeLines[(int) LaneSide::LEFT] =
		BiarcPair::CreateParallel(m_dividerLines.front(),
		-GetInput().group->GetLeftShoulderWidth(),
		-GetOutput().group->GetLeftShoulderWidth());
	m_edgeLines[(int) LaneSide::RIGHT] =
		BiarcPair::CreateParallel(m_dividerLines.back(),
		GetInput().group->GetRightShoulderWidth(),
		GetOutput().group->GetRightShoulderWidth());
	if (GetTwin() != nullptr)
	{
		m_edgeLines[(int) LaneSide::LEFT] = m_dividerLines[0];
	}

	float h1 = GetInput().group->GetPosition().z;
	float h2 = GetOutput().group->GetPosition().z;
	float slope1 = GetInput().group->GetSlope();
	float slope2 = GetOutput().group->GetSlope();
	m_visualEdgeLines[0] = RoadCurveLine(m_dividerLines.front(), h1, h2, slope1, slope2);
	m_visualEdgeLines[1] = RoadCurveLine(m_dividerLines.back(), h1, h2, slope1, slope2);
	m_visualShoulderLines[0] = RoadCurveLine(m_edgeLines[0], h1, h2, slope1, slope2);
	m_visualShoulderLines[1] = RoadCurveLine(m_edgeLines[1], h1, h2, slope1, slope2);

	m_seams[0][0].clear();
	m_seams[0][1].clear();
	m_seams[1][0].clear();
	m_seams[1][1].clear();
}

