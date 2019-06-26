#include "NodeGroup.h"
#include "NodeGroupTie.h"
#include "NodeGroupConnection.h"
#include <algorithm>


//-----------------------------------------------------------------------------
// NodeSubGroup
//-----------------------------------------------------------------------------

NodeSubGroup::NodeSubGroup()
	: group(nullptr)
	, index(0)
	, count(0)
{
}

NodeSubGroup::NodeSubGroup(NodeGroup* group, int index, int count)
	: group(group)
	, index(index)
	, count(count)
{
}

float NodeSubGroup::GetWidth() const
{
	float width = 0.0f;
	for (int i = 0; i < count; i++)
		width += group->GetNode(index + i)->GetWidth();
	return width;
}

Vector3f NodeSubGroup::GetLeftPosition() const
{
	return group->GetNode(index)->GetPosition();
}

Vector3f NodeSubGroup::GetCenterPosition() const
{
	Vector3f leftEdge = group->GetNode(index)->GetPosition();
	float width = GetWidth();
	Vector2f right = RightPerpendicular(group->GetDirection());
	return Vector3f(leftEdge.xy + (right * width * 0.5f), leftEdge.z);
}
Node* NodeSubGroup::GetNode(int index)
{
	return group->GetNode(this->index + index);
}

bool NodeSubGroup::ContainsNode(Node* node)
{
	return (node->GetNodeGroup() == group &&
		node->GetIndex() >= index && 
		node->GetIndex() < index + count);
}


int NodeSubGroup::GetOverlap(const NodeSubGroup& a, const NodeSubGroup& b)
{
	return Math::Min(
		a.index + a.count - b.index,
		b.index + b.count - a.index);
}


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroup::NodeGroup()
	: m_metrics(nullptr)
	, m_tie(nullptr)
	, m_intersection(nullptr)
	, m_inputIntersection(nullptr)
	, m_position(Vector3f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_allowPassing(false)
	, m_rightOfWay(RightOfWay::NONE)
	, m_rightShoulderWidth(0.0f)
	, m_leftShoulderWidth(0.0f)
{
	//m_rightOfWay = Random::NextBool() ? RightOfWay::NONE : RightOfWay::GIVE_WAY;
}

NodeGroup::~NodeGroup()
{
	// Delete all nodes in the node group
	for (unsigned int i = 0; i < m_nodes.size(); i++)
		delete m_nodes[i];
	m_nodes.clear();
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

const Vector3f& NodeGroup::GetPosition() const
{
	return m_position;
}

const Vector2f& NodeGroup::GetDirection() const
{
	return m_direction;
}

Vector2f NodeGroup::GetLeftDirection() const
{
	return LeftPerpendicular(m_direction);
}

Vector2f NodeGroup::GetRightDirection() const
{
	return RightPerpendicular(m_direction);
}

const RoadMetrics* NodeGroup::GetMetrics() const
{
	return m_metrics;
}

NodeGroup* NodeGroup::GetTwin() const
{
	return m_twin;
}

RoadIntersection* NodeGroup::GetIntersection(IOType type) const
{
	if (type == IOType::OUTPUT)
		return m_intersection;
	return m_inputIntersection;
}

Node* NodeGroup::GetLeftNode() const
{
	if (m_nodes.empty())
		return nullptr;
	else
		return m_nodes.front();
}

Node* NodeGroup::GetRightNode() const
{
	if (m_nodes.empty())
		return nullptr;
	else
		return m_nodes.back();
}

Node* NodeGroup::GetNode(int index)
{
	return m_nodes[index];
}

NodeGroupTie* NodeGroup::GetTie()
{
	return m_tie;
}

int NodeGroup::GetNumNodes() const
{
	return (int) m_nodes.size();
}

Meters NodeGroup::GetWidth() const
{
	float width = 0.0f;
	for (unsigned int i = 0; i < m_nodes.size(); i++)
		width += m_nodes[i]->m_width;
	return width;
}

Meters NodeGroup::GetShoulderWidth(LaneSide side) const
{
	if (side == LaneSide::RIGHT)
		return m_rightShoulderWidth;
	else
		return m_leftShoulderWidth;

}

Meters NodeGroup::GetRightShoulderWidth() const
{
	return m_rightShoulderWidth;
}

Meters NodeGroup::GetLeftShoulderWidth() const
{
	return m_leftShoulderWidth;
}

Vector3f NodeGroup::GetCenterPosition() const
{
	float width = GetWidth();
	Vector2f right = RightPerpendicular(m_direction);
	return Vector3f(m_position.xy + (right * width * 0.5f), m_position.z);
}

Vector3f NodeGroup::GetRightPosition() const
{
	float width = GetWidth();
	Vector2f right = RightPerpendicular(m_direction);
	return Vector3f(m_position.xy + (right * width), m_position.z);
}

Array<NodeGroupConnection*>& NodeGroup::GetInputs()
{
	return m_connections[(int) InputOutput::INPUT];
}

Array<NodeGroupConnection*>& NodeGroup::GetOutputs()
{
	return m_connections[(int) InputOutput::OUTPUT];
}

bool NodeGroup::IsTied() const
{
	return (m_tie != nullptr);
}

RightOfWay NodeGroup::GetRightOfWay() const
{
	return m_rightOfWay;
}

float NodeGroup::GetSlope() const
{
	return m_slope;
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void NodeGroup::SetPosition(const Vector3f& position)
{
	m_position = position;
}

void NodeGroup::SetAltitude(float z)
{
	m_position.z = z;
}

void NodeGroup::SetDirection(const Vector2f& direction)
{
	m_direction = direction;
}

void NodeGroup::SetDirectionFromCenter(const Vector2f& direction)
{
	float width = GetWidth();
	Vector2f right = RightPerpendicular(m_direction);
	Vector2f center(m_position.xy + (right * width * 0.5f));
	m_direction = direction;
	right = RightPerpendicular(direction);
	m_position.xy = center - (right * width * 0.5f);
}

void NodeGroup::SetRightOfWay(RightOfWay rightOfWay)
{
	m_rightOfWay = rightOfWay;
}


//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------

static bool IsPointOnArc(const Vector2f& v, const Biarc& arc)
{
	float x = (v - arc.center).Dot(arc.GetStartNormal()) / arc.radius;
	float y = (v - arc.center).Dot(arc.GetStartTangent()) / arc.radius;
	float angle = Math::ATan2(y, x);
	if (angle < 0)
		angle = Math::TWO_PI + angle;
	return (angle <= Math::Abs(arc.angle));
}

static bool IsPointOnArcs(const Vector2f& v, const Biarc& a, const Biarc& b)
{
	return (IsPointOnArc(v, a) && IsPointOnArc(v, b));
}

static bool IntersectArcs(Biarc& a, Biarc& b, Biarc& seam)
{
	if (a.IsPoint() || b.IsPoint())
		return false;

	// Compute the intersection of two circles:
	// http://mathworld.wolfram.com/Circle-CircleIntersection.html
	Vector2f c1 = a.center;
	Vector2f c2 = b.center;
	float r1sqr = a.radius * a.radius;
	float r2sqr = b.radius * b.radius;
	float distSqr = c1.DistToSqr(c2);
	float dist = Math::Sqrt(distSqr);
	float x = (distSqr - r2sqr + r1sqr) / (2 * dist);
	float discriminant = 4 * distSqr * r1sqr -
		Math::Sqr(distSqr - r2sqr + r1sqr);
	if (discriminant < 0.0f)
		return false;

	// Compute the intersection point
	float y = (0.5f / dist) * Math::Sqrt(discriminant);
	Vector2f mid = c1 + (c2 - c1) * (x / dist);
	Vector2f dir = RightPerpendicular((c1 - c2) / dist);
	Vector2f intersection = mid + (dir * y);

	// Check if the intersection point is on both arcs
	// Also try the alternate circle intersection point
	if (!IsPointOnArcs(intersection, a, b))
	{
		intersection = mid - (dir * y);
		if (!IsPointOnArcs(intersection, a, b))
			return false;
	}

	// Modify the arcs to start at the intersection point
	seam.start = a.start;
	seam.center = a.center;
	seam.radius = a.radius;
	//seam = a;
	seam.end = intersection;
	b.start = intersection;
	b.CalcAngleAndLength(true);
	seam.CalcAngleAndLength(true);
	a.start = intersection;
	a.CalcAngleAndLength(true);
	return true;
}

static bool IntersectArcPairs(RoadCurveLine& pair1, RoadCurveLine& pair2, bool reverse, RoadCurveLine& seam)
{
	if (reverse)
	{
		pair1 = pair1.Reverse();
		pair2 = pair2.Reverse();
		bool result = IntersectArcPairs(pair1, pair2, false, seam);
		seam = seam.Reverse();
		pair1 = pair1.Reverse();
		pair2 = pair2.Reverse();
		return result;
	}
	RoadCurveLine originalPair1 = pair1;
	RoadCurveLine originalPair2 = pair2;
	seam.verticalCurve = pair1.verticalCurve;

	if (IntersectArcs(pair1.horizontalCurve.second,
		pair2.horizontalCurve.second, seam.horizontalCurve.second))
	{
		pair1.horizontalCurve.first = Biarc::CreatePoint(
			pair1.horizontalCurve.second.start);
		pair2.horizontalCurve.first = pair1.horizontalCurve.first;
		seam.horizontalCurve.first = originalPair1.horizontalCurve.first;
	}
	else if (IntersectArcs(pair1.horizontalCurve.first,
		pair2.horizontalCurve.second, seam.horizontalCurve.first))
	{
		pair2.horizontalCurve.first = Biarc::CreatePoint(
			pair2.horizontalCurve.second.start);
		seam.horizontalCurve.second = Biarc::CreatePoint(seam.horizontalCurve.first.end);
	}
	else if (IntersectArcs(pair1.horizontalCurve.second,
		pair2.horizontalCurve.first, seam.horizontalCurve.second))
	{
		pair1.horizontalCurve.first = Biarc::CreatePoint(
			pair1.horizontalCurve.second.start);
		seam.horizontalCurve.first = originalPair1.horizontalCurve.first;
	}
	else if (IntersectArcs(pair1.horizontalCurve.first,
		pair2.horizontalCurve.first, seam.horizontalCurve.first))
	{
		seam.horizontalCurve.second = Biarc::CreatePoint(seam.horizontalCurve.first.end);
	}
	else
	{
		return false;
	}

	pair2.verticalCurve.height1 = pair1.verticalCurve.GetHeightFromDistance(
		seam.horizontalCurve.Length());
	pair2.verticalCurve.slope1 = pair1.verticalCurve.GetSlope(
		seam.horizontalCurve.Length());
	pair2.verticalCurve.CubicInterpolatation(
		pair2.verticalCurve.slope1,
		pair2.verticalCurve.slope2,
		pair2.horizontalCurve.Length());
	//pair2.verticalCurve.Slice(pair2.Length() - originalPair2.Length());
	pair1.verticalCurve.Slice(seam.Length());
	seam.verticalCurve.length = seam.horizontalCurve.Length();
	//pair2.verticalCurve.height1 = pair1.verticalCurve.GetHeight(seam.Length() / originalPair1.Length());
	//pair1.verticalCurve.height1 = pair2.verticalCurve.height1;
	//seam.verticalCurve.height2 = pair2.verticalCurve.height1;
	//pair1.t1 = Math::Lerp(pair1.t2, pair1.t1, pair1.Length() / originalPair1.Length());
	//pair2.t1 = Math::Lerp(pair2.t2, pair2.t1, pair2.Length() / originalPair2.Length());
	return true;
}

bool NodeGroup::IntersectConnections(
	NodeGroupConnection* a, NodeGroupConnection* b, IOType end)
{
	RoadCurveLine seam;
	bool reverse = (end == IOType::INPUT);
	IOType otherEnd = (end == IOType::INPUT ? IOType::OUTPUT : IOType::INPUT);

	if (IntersectArcPairs(
		a->m_visualShoulderLines[(int) LaneSide::LEFT],
		b->m_visualShoulderLines[(int) LaneSide::RIGHT],
		reverse, seam))
	{
		a->AddSeam(otherEnd, LaneSide::LEFT, seam);
		b->SetSeam(otherEnd, LaneSide::RIGHT, seam);
		return true;
	}
	else if (IntersectArcPairs(
		a->m_visualShoulderLines[(int) LaneSide::RIGHT],
		b->m_visualShoulderLines[(int) LaneSide::LEFT],
		reverse, seam))
	{
		a->AddSeam(otherEnd, LaneSide::RIGHT, seam);
		b->SetSeam(otherEnd, LaneSide::LEFT, seam);
		return true;
	}

	return false;
}

bool NodeGroup::IntersectEdgeLines(
	NodeGroupConnection* a, NodeGroupConnection* b, IOType end)
{
	RoadCurveLine seam;
	bool reverse = (end == IOType::INPUT);
	IOType otherEnd = (end == IOType::INPUT ? IOType::OUTPUT : IOType::INPUT);

	if (IntersectArcPairs(
		a->m_visualDividerLines.front(),
		b->m_visualDividerLines.back(),
		reverse, seam))
	{
		a->AddEdgeSeam(otherEnd, LaneSide::LEFT, seam);
		b->SetEdgeSeam(otherEnd, LaneSide::RIGHT, seam);
		return true;
	}
	else if (IntersectArcPairs(
		a->m_visualDividerLines.back(),
		b->m_visualDividerLines.front(),
		reverse, seam))
	{
		a->AddEdgeSeam(otherEnd, LaneSide::RIGHT, seam);
		b->SetEdgeSeam(otherEnd, LaneSide::LEFT, seam);
		return true;
	}

	return false;
}

void NodeGroup::UpdateGeometry()
{
	// Adjust the node positions relative to the node group's center
	Vector3f nodePosition = m_position;
	Vector2f right = RightPerpendicular(m_direction);
	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		Node* node = m_nodes[i];
		node->m_position = nodePosition;
		node->m_direction = m_direction;
		nodePosition.xy += node->m_width * right;
	}

	// Determine the slope
	m_slope = 0.0f;
	if (!m_connections[0].empty() && !m_connections[1].empty())
	{
		// The node group's slope will be most gradual linear slope
		// of all its connections
		m_slope = m_connections[1][0]->GetLinearSlope();
		bool up = false;
		bool down = false;
		for (unsigned int side = 0; side < 2; side++)
		{
			for (unsigned int i = 0; i < m_connections[side].size(); i++)
			{
				float connectionSlope = m_connections[side][i]->GetLinearSlope();
				if (connectionSlope > 0)
					up = true;
				else
					down = true;
				if (Math::Abs(connectionSlope) < Math::Abs(m_slope))
					m_slope = connectionSlope;
			}
		}

		// If grade direction changes between any connections then use a flat slope
		if (up && down)
			m_slope = 0.0f;
	}
}

void NodeGroup::UpdateIntersectionGeometry()
{
	unsigned int first, last, i, j;
	RoadCurveLine seam;

	// Intersect shoulder edges between tied connections
	if (m_twin != nullptr)
	{
		auto outputs = GetOutputs();
		for (unsigned int i = 0; i < outputs.size() &&
			outputs[i]->GetInput().index == 0; i++)
		{
			NodeGroupConnection* a = outputs[i];
			for (unsigned int j = 0; j < m_twin->GetInputs().size() &&
				m_twin->GetInputs()[j]->GetOutput().index == 0; j++)
			{
				NodeGroupConnection* b = m_twin->GetInputs()[j];
				RoadCurveLine twinCurve = b->m_visualShoulderLines[
					(int) LaneSide::LEFT].Reverse();

				if (IntersectArcPairs(
					a->m_visualShoulderLines[(int) LaneSide::LEFT],
					twinCurve, false, seam))
				{
					//a->AddSeam(otherEnd, LaneSide::LEFT, seam);
					//b->SetSeam(otherEnd, LaneSide::RIGHT, seam);
				}
				b->m_visualShoulderLines[(int) LaneSide::LEFT] = twinCurve.Reverse();
				break;
			}
		}
	}

	// Check for overlap between neighboring connections
	for (int inOut = 0; inOut < 2; inOut++)
	{
		Array<NodeGroupConnection*>& connections = m_connections[inOut];
		bool reverse = (inOut == 0);
		// Intersect lane edges
		for (first = 0; first < connections.size(); first = last - 1)
		{
			// Find the group of connections that overlap this one
			for (last = first + 1; last < connections.size(); last++)
			{
				if (NodeSubGroup::GetOverlap(connections[first]->GetInput(),
					connections[last]->GetInput()) < 1)
					break;
			}

			if (last == first + 1)
			{
				last = first + 2;
				continue;
			}

			// Intersect the lane-edge arc geometry for the overlapping
			// connections
			for (i = first; i < last; i++)
			{
				NodeGroupConnection* a = connections[i];
				for (j = i + 1; j < last; j++)
				{
					NodeGroupConnection* b = connections[j];
					// TODO: reverse
					if (reverse)
						IntersectEdgeLines(a, b, (InputOutput) inOut);
					else
						IntersectEdgeLines(a, b, (InputOutput) inOut);
				}
			}
		}

		// Intersect shoulder edges
		for (first = 0; first < connections.size(); first = last - 1)
		{
			// Find the group of connections that are touching or overlapping
			for (last = first + 1; last < connections.size(); last++)
			{
				if (NodeSubGroup::GetOverlap(connections[first]->GetInput(),
					connections[last]->GetInput()) < 0)
					break;
			}

			if (last == first + 1)
			{
				last = first + 2;
				continue;
			}


			// Intersect the shoulder-edge arc geometry for the touching
			// connections
			for (i = first; i < last; i++)
			{
				for (j = i + 1; j < last; j++)
				{
					IntersectConnections(connections[i],
						connections[j], (IOType) inOut);
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Internal Methods
//-----------------------------------------------------------------------------

void NodeGroup::InsertInput(NodeGroupConnection* input)
{
	InsertConnection(input, 0);
}

void NodeGroup::InsertOutput(NodeGroupConnection* output)
{
	InsertConnection(output, 1);
}

void NodeGroup::InsertConnection(NodeGroupConnection* connection, int direction)
{
	Array<NodeGroupConnection*>& connections = m_connections[direction];
	int opposite = 1 - direction;

	// Insert the connection in sorted order
	// Left-most lanes come first
	for (unsigned int i = 0; i < connections.size(); i++)
	{
		if (connection->m_groups[opposite].index <=
			connections[i]->m_groups[opposite].index &&
			connection->m_groups[opposite].count <=
			connections[i]->m_groups[opposite].count &&
			connections[i]->GetTwin() == nullptr)
		{
			connections.insert(connections.begin() + i, connection);
			return;
		}
	}

	connections.push_back(connection);
}

void NodeGroup::UpdateConnectionSorting(bool search)
{
	// Connections with twins must be FIRST in the list so their edges
	// do not get clipped, but instead they clip other connections.
	for (int direction = 0; direction < 2; direction++)
	{
		Array<NodeGroupConnection*>& connections = m_connections[direction];
		for (unsigned int i = 0; i < connections.size(); i++)
		{
			NodeGroupConnection* connection = connections[i];
			if (connection->GetSubGroup((IOType) (1 - direction)).index != 0)
				break;
			if (connection->GetTwin() != nullptr)
			{
				connections.erase(connections.begin() + i);
				connections.insert(connections.begin(), connection);
				if (search)
				{
					connection->GetSubGroup((IOType) direction).group->
						UpdateConnectionSorting(false);
				}
				break;
			}
		}
	}
}

void NodeGroup::RemoveInput(NodeGroupConnection* input)
{
	RemoveConnection(input, 0);
}

void NodeGroup::RemoveOutput(NodeGroupConnection* output)
{
	RemoveConnection(output, 1);
}

void NodeGroup::RemoveConnection(NodeGroupConnection* connection, int direction)
{
	Array<NodeGroupConnection*>& connections = m_connections[direction];
	auto it = std::find(connections.begin(), connections.end(), connection);
	if (it != connections.end())
		connections.erase(it);
}
