#include "RoadIntersection.h"
#include "NodeGroupConnection.h"
#include <algorithm>


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

RoadIntersection::RoadIntersection()
{
}

RoadIntersection::~RoadIntersection()
{
	for (unsigned int i = 0; i < m_points.size(); i++)
		delete m_points[i];
	m_points.clear();
	for (unsigned int i = 0; i < m_edges.size(); i++)
		delete m_edges[i];
	m_edges.clear();
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

Vector2f RoadIntersection::GetCenterPosition() const
{
	return m_centerPosition;
}

Array<RoadIntersectionPoint*>& RoadIntersection::GetPoints()
{
	return m_points;
}

Array<RoadIntersectionEdge*>& RoadIntersection::GetEdges()
{
	return m_edges;
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void RoadIntersection::Construct(const Set<NodeGroup*>& nodeGroups)
{
	for (unsigned int i = 0; i < m_points.size(); i++)
		delete m_points[i];
	m_points.clear();
	for (unsigned int i = 0; i < m_edges.size(); i++)
		delete m_edges[i];
	m_edges.clear();
	m_nodeGroups = nodeGroups;

	// Get the center position of all node groups
	m_centerPosition = Vector2f::ZERO;
	int count = 0;
	for (NodeGroup* group : m_nodeGroups)
	{
		group->m_intersection = this;
		AddPoint(group, group->GetOutputs().empty() ? IOType::INPUT : IOType::OUTPUT);
		if (group->m_twin == nullptr)
		{
			m_centerPosition += group->GetCenterPosition().xy;
			count++;
		}
		else
		{
			m_centerPosition += group->GetPosition().xy * 2.0f;
			count += 2;
		}
	}
	m_centerPosition /= (float) count;

	// Determine the angles of each group around the center
	std::map<RoadIntersectionPoint*, float> groupAngles;
	for (RoadIntersectionPoint* point : m_points)
	{
		Vector2f v = point->GetNodeGroup()->GetCenterPosition().xy -
			m_centerPosition;
		groupAngles[point] = Math::ATan2(v.y, v.x);
	}

	// Sort the node groups in counter-clockwise order around the center
	// position
	std::sort(m_points.begin(), m_points.end(),
		[&](RoadIntersectionPoint* a, RoadIntersectionPoint* b) -> bool {
		return (groupAngles[a] > groupAngles[b]);
	});

	// Create the edges
	for (unsigned int i = 0; i < m_points.size(); i++)
	{
		RoadIntersectionPoint* left = m_points[(i + 1) % m_points.size()];
		RoadIntersectionPoint* right = m_points[i];
		if (left->GetNodeGroup()->GetTwin() != right->GetNodeGroup())
			m_edges.push_back(new RoadIntersectionEdge(left, right));
	}
}

RoadIntersectionPoint* RoadIntersection::AddPoint(NodeGroup* group, IOType type)
{
	RoadIntersectionPoint* point = new RoadIntersectionPoint(group, type);
	m_points.push_back(point);
	return point;
}


//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------

void RoadIntersection::UpdateGeometry()
{
	// Calculate the center position of all node groups
	m_centerPosition = Vector2f::ZERO;
	int count = 0;
	for (RoadIntersectionPoint* point : m_points)
	{
		if (point->GetNodeGroup()->GetTwin() == nullptr)
		{
			m_centerPosition += point->GetNodeGroup()->GetCenterPosition().xy;
			count++;
		}
		else
		{
			m_centerPosition += point->GetNodeGroup()->GetPosition().xy * 2.0f;
			count += 2;
		}
	}
	m_centerPosition /= (float) count;

	// Create curves for the edges between node groups
	for (unsigned int i = 0; i < m_edges.size(); i++)
	{
		RoadIntersectionEdge* edge = m_edges[i];
		BiarcPair arcs[2];
		//BiarcPair* edgeLines[2];
		RoadIntersectionPoint* points[2];
		NodeGroupConnection* connections[2];
		bool reverse[2];
		LaneSide sides[2];

		points[0] = edge->m_points[0];
		points[1] = edge->m_points[1];
		if (points[0]->m_nodeGroup->GetTwin() == points[1]->m_nodeGroup)
			continue;
		sides[0] = (points[0]->m_ioType == IOType::INPUT ?
			LaneSide::RIGHT : LaneSide::LEFT);
		sides[1] = (points[1]->m_ioType == IOType::INPUT ?
			LaneSide::LEFT : LaneSide::RIGHT);

		for (int k = 0; k < 2; k++)
		{
			connections[k] = points[k]->GetConnection(sides[k]);
			arcs[k] = connections[k]->m_edgeLines[(int) sides[k]];
			reverse[k] = (points[k]->GetIOType() == IOType::OUTPUT);
			if (reverse[k])
				arcs[k] = arcs[k].Reverse();
			//edgeLines[k] = &connections[k]->m_visualShoulderLines[(int) sides[k]];
		}

		BiarcPair shoulderEdge;
		shoulderEdge = CalcWebbedCircle(arcs[0], arcs[1], 4.5f);

		BiarcPair laneEdge;
		float offset = points[0]->m_nodeGroup->GetShoulderWidth(sides[0]);
		laneEdge = BiarcPair::CreateParallel(shoulderEdge, -offset);

		//for (int k = 0; k < 2; k++)
		//{
		//	arcs[k].second.end = shoulderEdges[k].start;
		//	arcs[k].second.CalcAngleAndLength(true);
		//	if (reverse[k])
		//		arcs[k] = arcs[k].Reverse();
		//	*edgeLines[k] = arcs[k];
		//}

		edge->m_shoulderEdge = shoulderEdge;
		edge->m_laneEdge  = laneEdge;
		if (shoulderEdge.first.IsStraight())
		{
			edge->m_line = &edge->m_shoulderEdge.first;
			edge->m_arc = &edge->m_shoulderEdge.second;
		}
		else
		{
			edge->m_arc = &edge->m_shoulderEdge.first;
			edge->m_line = &edge->m_shoulderEdge.second;
		}

		//edge.m_arc.
		BiarcPair split = BiarcPair::Split(*edge->m_arc);
		edge->m_halfArcs[0] = split.first;
		edge->m_halfArcs[1] = split.second.Reverse();
	}

	//for (unsigned int i = 0; i < m_edges.size(); i++)
	//{
	//	RoadIntersectionEdge& left = m_edges[(i + 1) % m_edges.size()];
	//	RoadIntersectionEdge& right = m_edges[i];


	//}
}
