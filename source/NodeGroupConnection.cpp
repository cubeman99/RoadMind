#include "NodeGroupConnection.h"
#include "NodeGroupTie.h"
#include "Geometry.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroupConnection::NodeGroupConnection()
	: m_metrics(nullptr)
	, m_isGhost(false)
	, m_mesh(nullptr)
{
	m_mesh = new Mesh();
}

NodeGroupConnection::~NodeGroupConnection()
{
	delete m_mesh;
	m_mesh = nullptr;
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

const RoadCurveLine& NodeGroupConnection::GetVisualEdgeLine(LaneSide side) const
{
	if (side == LaneSide::LEFT)
		return m_visualDividerLines.front();
	else
		return m_visualDividerLines.back();
}

const RoadCurveLine& NodeGroupConnection::GetLeftVisualEdgeLine() const
{
	return m_visualDividerLines.front();
}

const RoadCurveLine& NodeGroupConnection::GetRightVisualEdgeLine() const
{
	return m_visualDividerLines.back();
}

int NodeGroupConnection::GetDividerLineCount() const
{
	return (int) m_visualDividerLines.size();
}

const RoadCurveLine& NodeGroupConnection::GetVisualDividerLine(int index) const
{
	return m_visualDividerLines[index];
}

const RoadCurveLine& NodeGroupConnection::GetLeftVisualShoulderLine() const
{
	return m_visualShoulderLines[(int) LaneSide::LEFT];
}

const RoadCurveLine& NodeGroupConnection::GetRightVisualShoulderLine() const
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

const Array<RoadCurveLine>& NodeGroupConnection::GetEdgeSeams(IOType type, LaneSide side) const
{
	return m_edgeSeams[(int) type][(int) side];
}

Array<RoadCurveLine>& NodeGroupConnection::GetEdgeSeams(IOType type, LaneSide side)
{
	return m_edgeSeams[(int) type][(int) side];
}

RoadCurveLine NodeGroupConnection::GetDrivingLine(int fromLaneIndex, int toLaneIndex)
{
	// Get the endpoint offsets from the left-most lane edge
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

	// Create the horizontal and vertical curve
	BiarcPair horizontal = BiarcPair::CreateParallel(
		m_leftLaneEdge.horizontalCurve,
		offsets[0] - (widths[0] * 0.5f),
		offsets[1] - (widths[1] * 0.5f));
	Meters h1 = GetInput().group->GetPosition().z;
	Meters h2 = GetOutput().group->GetPosition().z;
	float slope1 = GetInput().group->GetSlope();
	float slope2 = GetOutput().group->GetSlope();
	return RoadCurveLine(horizontal, h1, h2, slope1, slope2);
}

RoadCurveLine NodeGroupConnection::GetDrivingLine(int laneIndex)
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
	if (m_visualDividerLines.empty())
		return 0.0f;
	Vector3f center0 = m_groups[0].GetCenterPosition();
	Vector3f center1 = m_groups[1].GetCenterPosition();
	Meters dy = center1.z - center0.z;
	Meters dx = (GetLeftVisualEdgeLine().Length() +
		GetRightVisualEdgeLine().Length()) * 0.5f;
	if (dx < FLT_EPSILON)
		return 0.0f;
	else
		return dy / dx;
}

Mesh* NodeGroupConnection::GetMesh()
{
	return m_mesh;
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

void NodeGroupConnection::SetEdgeSeam(IOType end, LaneSide side, const RoadCurveLine& seam)
{
	Array<RoadCurveLine>& seams = GetEdgeSeams(end, side);
	seams.resize(1);
	seams[0] = seam;
}

void NodeGroupConnection::AddEdgeSeam(IOType end, LaneSide side, const RoadCurveLine& seam)
{
	Array<RoadCurveLine>& seams = GetEdgeSeams(end, side);
	seams.push_back(seam);
}

void NodeGroupConnection::UpdateGeometry()
{
	m_visualDividerLines.clear();
	for (unsigned int i = 0; i < 2; i++)
	{
		for (unsigned int j = 0; j < 2; j++)
		{
			m_seams[i][j].clear();
			m_edgeSeams[i][j].clear();
		}
	}

	// Create the left edge
	Node* nodes[2];
	nodes[0] = GetInput().group->GetNode(GetInput().index);
	nodes[1] = GetOutput().group->GetNode(GetOutput().index);
	BiarcPair prev, curr;
	prev = BiarcPair::Interpolate(
		nodes[0]->m_position.xy, GetInput().group->GetDirection(),
		nodes[1]->m_position.xy, GetOutput().group->GetDirection());
	m_visualDividerLines.push_back(RoadCurveLine(prev));

	// Determine side with less nodes
	int side1 = m_groups[1].count >= m_groups[0].count ? 0 : 1;
	int side2 = 1 - side1;
	int count1 = m_groups[side1].count;
	int count2 = m_groups[side2].count;
	NodeSubGroup& group1 = m_groups[side1];
	NodeSubGroup& group2 = m_groups[side2];

	float scale = 1.0f;
	if (side1 == 1)
	{
		m_visualDividerLines[0] = m_visualDividerLines[0].Reverse();
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
		m_visualDividerLines.push_back(RoadCurveLine(
			BiarcPair::CreateParallel(
				m_visualDividerLines[i].horizontalCurve,
				w0 * scale, w1 * scale)));
	}
	if (side1 == 1)
	{
		for (unsigned int i = 0; i < m_visualDividerLines.size(); i++)
			m_visualDividerLines[i] = m_visualDividerLines[i].Reverse();
	}

	float h1 = GetInput().group->GetPosition().z;
	float h2 = GetOutput().group->GetPosition().z;
	float slope1 = GetInput().group->GetSlope();
	float slope2 = GetOutput().group->GetSlope();

	// Interpolate vertical curves for lane dividers
	for (unsigned int i = 0; i < m_visualDividerLines.size(); i++)
	{
		m_visualDividerLines[i].verticalCurve = VerticalCurve(
			h1, h2, m_visualDividerLines[i].horizontalCurve.Length(), slope1, slope2);
	}
	m_visualEdgeLines[0] = &m_visualDividerLines[0];
	m_visualEdgeLines[1] = &m_visualDividerLines[m_visualDividerLines.size() - 1];
	m_leftLaneEdge = m_visualDividerLines[0];

	// Create the left and right road surface edges (shoulder edges)
	m_visualShoulderLines[0].horizontalCurve =
		BiarcPair::CreateParallel(m_visualDividerLines.front().horizontalCurve,
			-GetInput().group->GetLeftShoulderWidth(),
			-GetOutput().group->GetLeftShoulderWidth());
	m_visualShoulderLines[1].horizontalCurve =
		BiarcPair::CreateParallel(m_visualDividerLines.back().horizontalCurve,
			GetInput().group->GetRightShoulderWidth(),
			GetOutput().group->GetRightShoulderWidth());
	if (GetTwin() != nullptr)
	{
		m_visualShoulderLines[(int) LaneSide::LEFT] = m_visualDividerLines.front();
	}
	m_visualShoulderLines[0].verticalCurve = VerticalCurve(
		h1, h2, m_visualShoulderLines[0].horizontalCurve.Length(), slope1, slope2);
	m_visualShoulderLines[1].verticalCurve = VerticalCurve(
		h1, h2, m_visualShoulderLines[1].horizontalCurve.Length(), slope1, slope2);
}

void NodeGroupConnection::CreateMesh()
{

	NodeGroupConnection* twin = GetTwin();
	RoadCurveLine leftEdge;
	RoadCurveLine rightEdge = GetRightVisualShoulderLine();
	bool dominantTwin = true;
	if (twin != nullptr && GetId() > twin->GetId())
		dominantTwin = false;

	auto seamsIL = GetEdgeSeams(IOType::INPUT, LaneSide::LEFT);
	auto seamsIR = GetEdgeSeams(IOType::INPUT, LaneSide::RIGHT);
	auto seamsOL = GetEdgeSeams(IOType::OUTPUT, LaneSide::LEFT);
	auto seamsOR = GetEdgeSeams(IOType::OUTPUT, LaneSide::RIGHT);
	Array<RoadCurveLine> leftContour;
	Array<RoadCurveLine> rightContour;
	leftContour.resize(1);
	rightContour.resize(1);
	Array<VertexPosNorm> vertices;
	Array<unsigned int> indices;

	// Right shoulder
	leftContour[0] = m_visualDividerLines.back();
	rightContour[0] = m_visualShoulderLines[1];
	Geometry::ZipArcs(vertices, indices, leftContour, rightContour);

	// Left shoulder
	if (twin == nullptr)
	{
		leftContour[0] = m_visualShoulderLines[0];
		rightContour[0] = m_visualDividerLines[0];
		Geometry::ZipArcs(vertices, indices, leftContour, rightContour);
	}

	// Lane surface
	leftContour.clear();
	for (auto it = seamsIL.begin(); it != seamsIL.end(); it++)
		leftContour.push_back(*it);
	leftContour.push_back(GetLeftVisualEdgeLine());
	for (auto it = seamsOL.begin(); it != seamsOL.end(); it++)
		leftContour.push_back(*it);
	rightContour.clear();
	for (auto it = seamsIR.begin(); it != seamsIR.end(); it++)
		rightContour.push_back(*it);
	rightContour.push_back(GetRightVisualEdgeLine());
	for (auto it = seamsOR.begin(); it != seamsOR.end(); it++)
		rightContour.push_back(*it);
	Geometry::ZipArcs(vertices, indices, leftContour, rightContour);

	m_mesh->GetVertexData()->BufferVertices(vertices);
	m_mesh->GetIndexData()->BufferIndices(indices);
	m_mesh->SetIndices(0, indices.size());
	return;
}
