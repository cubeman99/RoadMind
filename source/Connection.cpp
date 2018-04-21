#include "Connection.h"
#include "Node.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Connection::Connection(Node* from, Node* to)
	: m_input(from)
	, m_output(to)
{
}

Connection::~Connection()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

Node* Connection::GetInput()
{
	return m_input;
}

Node* Connection::GetOutput()
{
	return m_output;
}

const Array<Vector2f>& Connection::GetVertices(LaneSide side) const
{
	return m_vertices[(int) side];
}

Connection* Connection::GetLeftConnection() const
{
	if (m_input->m_leftNode != nullptr &&
		m_output->m_leftNode != nullptr)
	{
		if (m_input->m_leftNode->HasInput(m_output->m_leftNode))
			return m_input->m_leftNode->GetInputConnection(m_output->m_leftNode);
		else if (m_output->m_leftNode->HasInput(m_input->m_leftNode))
			return m_output->m_leftNode->GetInputConnection(m_input->m_leftNode);
	}
	return nullptr;
}

Connection* Connection::GetRightConnection() const
{
	if (m_input->m_rightNode != nullptr &&
		m_output->m_rightNode != nullptr)
	{
		if (m_input->m_rightNode->HasInput(m_output->m_rightNode))
			return m_input->m_rightNode->GetInputConnection(m_output->m_rightNode);
		else if (m_output->m_rightNode->HasInput(m_input->m_rightNode))
			return m_output->m_rightNode->GetInputConnection(m_input->m_rightNode);
	}
	return nullptr;
}

bool Connection::IsLeftMostLane() const
{
	Connection* left = GetLeftConnection();
	return (left == nullptr || left->GetLeftConnection() == this);
}

bool Connection::IsRightMostLane() const
{
	return (GetRightConnection() == nullptr);
}

LaneDivider Connection::GetLaneDivider(LaneSide side) const
{
	if (side == LaneSide::LEFT)
	{
		Connection* left = GetLeftConnection();
		if (left != nullptr)
			return m_input->m_leftDivider;
		else
			return LaneDivider::SOLID;
	}
	else
	{
		Connection* right = GetRightConnection();
		if (right != nullptr)
			return right->GetInput()->m_leftDivider;
		else
			return LaneDivider::SOLID;
	}
}

float Connection::GetDistance() const
{
	return (m_centerArc1.length + m_centerArc2.length);
}

Vector2f Connection::GetPoint(Meters distance, LaneSide side, Meters offset)
{
	distance = Math::Clamp(distance, 0.0f,
		m_leftArc1.length + m_leftArc2.length);

	Vector2f point;
	Biarc centerArc;
	if (distance < m_leftArc1.length)
	{
		centerArc = m_leftArc1;
	}
	else
	{
		centerArc = m_leftArc2;
		distance -= m_leftArc1.length;
	}
	point = centerArc.GetPoint(distance);

	//if (side == LaneSide::LEFT)
	//{
	//	return point;
	//}
	//else
	{
		Vector2f normal;
		float offsetFromCenter = offset;
		if (side == LaneSide::RIGHT)
			offsetFromCenter = m_input->GetWidth() - offsetFromCenter;
		else if (side == LaneSide::CENTER)
			offsetFromCenter = (m_input->GetWidth() * 0.5f) - offsetFromCenter;
		if (centerArc.radius == 0.0f)
		{
			normal = (centerArc.end - centerArc.start) / centerArc.length;
			normal = Vector2f(-normal.y, normal.x);
		}
		else
		{
			normal = (centerArc.center - centerArc.start);
			normal = Vector2f(-normal.y, normal.x);
			if (centerArc.end.Dot(normal) < centerArc.center.Dot(normal))
				offsetFromCenter = -offsetFromCenter;
			normal = (point - centerArc.center) / centerArc.radius;
		}
		point += normal * offsetFromCenter;
		return point;
	}

	/*
	distance = Math::Clamp(distance, 0.0f,
	m_centerArc1.length + m_centerArc2.length);

	Vector2f point;
	Biarc centerArc;
	if (distance < m_centerArc1.length)
	{
	centerArc = m_centerArc1;
	}
	else
	{
	centerArc = m_centerArc2;
	distance -= m_centerArc1.length;
	}
	point = centerArc.GetPoint(distance);

	if (side == LaneSide::CENTER)
	{
	return point;
	}
	else
	{
	Vector2f normal;
	float offsetFromCenter = offset - (m_input->GetWidth() * 0.5f);
	if (side == LaneSide::RIGHT)
	offsetFromCenter = -offsetFromCenter;
	if (centerArc.radius == 0.0f)
	{
	normal = (centerArc.end - centerArc.start) / centerArc.length;
	normal = Vector2f(-normal.y, normal.x);
	}
	else
	{
	normal = (centerArc.center - centerArc.start);
	normal = Vector2f(-normal.y, normal.x);
	if (centerArc.end.Dot(normal) < centerArc.center.Dot(normal))
	offsetFromCenter = -offsetFromCenter;
	normal = (point - centerArc.center) / centerArc.radius;
	}
	point += normal * offsetFromCenter;
	return point;
	}
	*/
}

static void AddArcVertices(const Biarc& arc, Array<Vector2f>* vertices)
{
	int count = 10;

	if (arc.radius == 0.0f)
	{
		for (int j = 0; j < count; j++)
		{
			float t = j / (float) count;
			vertices->push_back(Vector2f::Lerp(arc.start, arc.end, t));
		}
	}
	else
	{
		Vector2f v = arc.start;
		float angle = arc.angle / (float) count;
		for (int j = 0; j < count; j++)
		{
			vertices->push_back(v);
			v.Rotate(arc.center, angle);
		}
	}
}


void Connection::CalcVertices()
{
	m_vertices[0].clear();
	m_vertices[1].clear();
	m_vertices[2].clear();

	Node* node1 = m_input;
	Node* node2 = m_output;

	float minWidth = Math::Min(node1->GetWidth(), node2->GetWidth());
	float maxWidth = Math::Min(node1->GetWidth(), node2->GetWidth());
	float widthExpansion = maxWidth - minWidth;
	float halfWidthExpansion = widthExpansion * 0.5f;
	bool flip = (widthExpansion < 0.0f);

	Connection* leftConnection = GetLeftConnection();
	if (leftConnection != nullptr)
	{
		if (leftConnection->GetLeftConnection() == this &&
			m_input->GetNodeId() < leftConnection->GetOutput()->m_nodeId)
		{
			// Compute biarcs for the left edge
			Vector2f p1, p2, t1, t2;
			p1 = node1->m_position;
			p2 = node2->m_position;
			t1 = node1->m_endNormal;
			t2 = node2->m_endNormal;
			ComputeBiarcs(p1, t1, p2, t2, m_leftArc1, m_leftArc2);
			AddArcVertices(m_leftArc1, &m_vertices[(int) LaneSide::LEFT]);
			AddArcVertices(m_leftArc2, &m_vertices[(int) LaneSide::LEFT]);
		}
		else
		{
			// Use biarcs from the twin connection
			m_leftArc1 = leftConnection->m_leftArc1;
			m_leftArc2 = leftConnection->m_leftArc2;
		}
	}

	if (flip)
	{
		//Node* temp = node2;
		//node2 = node1;
		//node1 = temp;
		//widthExpansion = -widthExpansion;
	}


	m_centerArc1 = CreateParallelBiarc(m_leftArc1, minWidth * 0.5f);
	m_centerArc2 = CreateParallelBiarc(m_leftArc2, minWidth * 0.5f);
	AddArcVertices(m_centerArc1, &m_vertices[(int) LaneSide::CENTER]);
	AddArcVertices(m_centerArc2, &m_vertices[(int) LaneSide::CENTER]);

	m_rightArc1 = CreateParallelBiarc(m_leftArc1, minWidth);
	m_rightArc2 = CreateParallelBiarc(m_leftArc2, minWidth);
	AddArcVertices(m_rightArc1, &m_vertices[(int) LaneSide::RIGHT]);
	AddArcVertices(m_rightArc2, &m_vertices[(int) LaneSide::RIGHT]);
	/*

	if (flip)
	{
	t1 = -t1;
	t2 = -t2;
	}
	if (!flip)
	{
	AddArcVertices(m_centerArc1, &m_vertices[2]);
	AddArcVertices(m_centerArc2, &m_vertices[2]);
	}
	else
	{
	AddArcVertices(m_centerArc2.Reverse(), &m_vertices[2]);
	AddArcVertices(m_centerArc1.Reverse(), &m_vertices[2]);
	}
	// Create left and right edges which are parellel to those biarcs
	Vector2f midPointLeftNormal = (m_centerArc1.end - m_centerArc1.center) / m_centerArc1.radius;
	if (m_centerArc1.angle < 0.0f)
	midPointLeftNormal = -midPointLeftNormal;

	// Left edge
	m_leftArc1 = CreateParallelBiarc(m_centerArc1, -halfWidth);
	m_leftArc2 = CreateParallelBiarc(m_centerArc2, -halfWidth);
	if (halfWidthExpansion > 0)
	{
	ComputeExpandingBiarcs(m_leftArc1.start, t1, m_leftArc2.end, t2, m_leftArc1.end,
	midPointLeftNormal, halfWidthExpansion, m_leftArc1, m_leftArc2);
	}
	if (!flip)
	{
	AddArcVertices(m_leftArc1, &m_vertices[(int) LaneSide::LEFT]);
	AddArcVertices(m_leftArc2, &m_vertices[(int) LaneSide::LEFT]);
	}
	else
	{
	AddArcVertices(m_leftArc2.Reverse(), &m_vertices[(int) LaneSide::RIGHT]);
	AddArcVertices(m_leftArc1.Reverse(), &m_vertices[(int) LaneSide::RIGHT]);
	}

	// Right edge
	m_rightArc1 = CreateParallelBiarc(m_centerArc1, halfWidth);
	m_rightArc2 = CreateParallelBiarc(m_centerArc2, halfWidth);
	if (halfWidthExpansion > 0)
	{
	ComputeExpandingBiarcs(m_rightArc1.start, t1, m_rightArc2.end, t2, m_rightArc1.end,
	midPointLeftNormal, -halfWidthExpansion, m_rightArc1, m_rightArc2);
	}
	if (!flip)
	{
	AddArcVertices(m_rightArc1, &m_vertices[(int) LaneSide::RIGHT]);
	AddArcVertices(m_rightArc2, &m_vertices[(int) LaneSide::RIGHT]);
	}
	else
	{
	AddArcVertices(m_rightArc2.Reverse(), &m_vertices[(int) LaneSide::LEFT]);
	AddArcVertices(m_rightArc1.Reverse(), &m_vertices[(int) LaneSide::LEFT]);
	}
	*/

	// Add the ending edge vertices
	m_vertices[(int) LaneSide::LEFT].push_back(m_output->GetLeftEdge());
	m_vertices[(int) LaneSide::RIGHT].push_back(m_output->GetRightEdge());
	m_vertices[2].push_back(m_output->GetCenter());
}

/*
void Connection::CalcVertices()
{
m_vertices[0].clear();
m_vertices[1].clear();
m_vertices[2].clear();

Node* node1 = m_input;
Node* node2 = m_output;

float minWidth = Math::Min(node1->GetWidth(), node2->GetWidth());
float halfWidth = minWidth * 0.5f;
float halfWidthExpansion = (node2->GetWidth() - node1->GetWidth()) * 0.5f;
bool flip = (halfWidthExpansion < 0.0f);

if (flip)
{
Node* temp = node2;
node2 = node1;
node1 = temp;
halfWidthExpansion = -halfWidthExpansion;
}

Vector2f p1, p2, t1, t2;
//Biarc m_centerArc1, m_centerArc2;
//Biarc m_leftArc1, m_leftArc2;
//Biarc m_rightArc1, m_rightArc2;

// TODO: Use left edge (side divider) as base arc

// Compute biarcs which pass through the lane center
p1 = node1->GetCenter();
p2 = node2->GetCenter();
t1 = node1->GetEndNormal();
t2 = node2->GetEndNormal();
if (flip)
{
t1 = -t1;
t2 = -t2;
}
ComputeBiarcs(p1, t1, p2, t2, m_centerArc1, m_centerArc2);
if (!flip)
{
AddArcVertices(m_centerArc1, &m_vertices[2]);
AddArcVertices(m_centerArc2, &m_vertices[2]);
}
else
{
AddArcVertices(m_centerArc2.Reverse(), &m_vertices[2]);
AddArcVertices(m_centerArc1.Reverse(), &m_vertices[2]);
}
// Create left and right edges which are parellel to those biarcs
Vector2f midPointLeftNormal = (m_centerArc1.end - m_centerArc1.center) / m_centerArc1.radius;
if (m_centerArc1.angle < 0.0f)
midPointLeftNormal = -midPointLeftNormal;

// Left edge
m_leftArc1 = CreateParallelBiarc(m_centerArc1, -halfWidth);
m_leftArc2 = CreateParallelBiarc(m_centerArc2, -halfWidth);
if (halfWidthExpansion > 0)
{
ComputeExpandingBiarcs(m_leftArc1.start, t1, m_leftArc2.end, t2, m_leftArc1.end,
midPointLeftNormal, halfWidthExpansion, m_leftArc1, m_leftArc2);
}
if (!flip)
{
AddArcVertices(m_leftArc1, &m_vertices[(int) LaneSide::LEFT]);
AddArcVertices(m_leftArc2, &m_vertices[(int) LaneSide::LEFT]);
}
else
{
AddArcVertices(m_leftArc2.Reverse(), &m_vertices[(int) LaneSide::RIGHT]);
AddArcVertices(m_leftArc1.Reverse(), &m_vertices[(int) LaneSide::RIGHT]);
}

// Right edge
m_rightArc1 = CreateParallelBiarc(m_centerArc1, halfWidth);
m_rightArc2 = CreateParallelBiarc(m_centerArc2, halfWidth);
if (halfWidthExpansion > 0)
{
ComputeExpandingBiarcs(m_rightArc1.start, t1, m_rightArc2.end, t2, m_rightArc1.end,
midPointLeftNormal, -halfWidthExpansion, m_rightArc1, m_rightArc2);
}
if (!flip)
{
AddArcVertices(m_rightArc1, &m_vertices[(int) LaneSide::RIGHT]);
AddArcVertices(m_rightArc2, &m_vertices[(int) LaneSide::RIGHT]);
}
else
{
AddArcVertices(m_rightArc2.Reverse(), &m_vertices[(int) LaneSide::LEFT]);
AddArcVertices(m_rightArc1.Reverse(), &m_vertices[(int) LaneSide::LEFT]);
}

// Add the ending edge vertices
m_vertices[(int) LaneSide::LEFT].push_back(m_output->GetLeftEdge());
m_vertices[(int) LaneSide::RIGHT].push_back(m_output->GetRightEdge());
m_vertices[2].push_back(m_output->GetCenter());
}
*/
