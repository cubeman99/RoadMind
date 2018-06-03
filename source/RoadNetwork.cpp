#include "RoadNetwork.h"
#include <map>
#include <algorithm>


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

RoadNetwork::RoadNetwork()
{
	m_nodeIdCounter = 1;
	m_nodeGroupConnectionIdCounter = 1;

	// Setup standard road metrics
	m_metrics.laneWidth = 3.7f;
	m_metrics.dividerWidth = 0.1f;
	m_metrics.dividerLength = 3.0f;
	m_metrics.dividerGapLength = 6.0f;
	m_metrics.stopLineWidth = 0.2f;
}

RoadNetwork::~RoadNetwork()
{
	ClearNodes();
}


//-----------------------------------------------------------------------------
// Topolgy Modifications
//-----------------------------------------------------------------------------

void RoadNetwork::ClearNodes()
{
	for (RoadIntersection* intersection : m_intersections)
		delete intersection;
	m_intersections.clear();
	for (NodeGroupConnection* surface : m_nodeGroupConnections)
		delete surface;
	m_nodeGroupConnections.clear();
	for (NodeGroupTie* tie : m_nodeGroupTies)
		delete tie;
	m_nodeGroupTies.clear();
	for (NodeGroup* nodeGroup : m_nodeGroups)
		delete nodeGroup;
	m_nodeGroups.clear();
}

NodeGroup* RoadNetwork::CreateNodeGroup(const Vector3f& position,
	const Vector2f& direction, int laneCount)
{
	// Construct the node group
	NodeGroup* group = new NodeGroup();
	group->m_metrics = &m_metrics;
	group->m_position = position;
	group->m_direction = direction;
	group->m_leftShoulderWidth = m_metrics.laneWidth * 0.25f;
	group->m_rightShoulderWidth = m_metrics.laneWidth * 0.25f;
	m_nodeGroups.insert(group);

	// Create the left-most node
	Node* node = CreateNode();
	node->m_nodeGroup = group;
	node->m_width = m_metrics.laneWidth;
	group->m_nodes.push_back(node);

	// Add nodes to the right
	Node* prev = node;
	for (int i = 1; i < laneCount; i++)
	{
		node = CreateNode();
		node->m_nodeGroup = group;
		node->m_width = m_metrics.laneWidth;
		prev = node;
		group->m_nodes.push_back(node);
	}

	return group;
}

RoadIntersection* RoadNetwork::CreateIntersection(
	const Set<NodeGroup*>& nodeGroups)
{
	RoadIntersection* intersection = new RoadIntersection();

	// Get the center position of all node groups
	Vector2f center = Vector2f::ZERO;
	int count = 0;
	for (NodeGroup* group : nodeGroups)
	{
		intersection->AddPoint(group,
			!group->GetOutputs().empty() ? IOType::OUTPUT : IOType::INPUT);
		if (group->m_twin == nullptr)
		{
			center += group->GetCenterPosition().xy;
			count++;
		}
		else
		{
			center += group->GetPosition().xy * 2.0f;
			count += 2;
		}
	}
	center /= (float) count;

	// Determine the angles of each group around the center
	std::map<RoadIntersectionPoint*, float> groupAngles;
	for (RoadIntersectionPoint* point : intersection->m_points)
	{
		Vector2f v = point->GetNodeGroup()->GetCenterPosition().xy - center;
		groupAngles[point] = Math::ATan2(v.y, v.x);
	}

	// Sort the node groups in counter-clockwise order around the center
	// position
	std::sort(intersection->m_points.begin(), intersection->m_points.end(),
		[&](RoadIntersectionPoint* a, RoadIntersectionPoint* b) -> bool {
		return (groupAngles[a] > groupAngles[b]);
	});

	// Create the edges
	for (unsigned int i = 0; i < intersection->m_points.size(); i++)
	{
		RoadIntersectionPoint* left = intersection->m_points[(i + 1) %
			intersection->m_points.size()];
		RoadIntersectionPoint* right = intersection->m_points[i];
		if (left->GetNodeGroup()->GetTwin() != right->GetNodeGroup())
			intersection->m_edges.push_back(new RoadIntersectionEdge(left, right));
	}

	m_intersections.insert(intersection);
	return intersection;
}

bool RoadNetwork::GrowNodeGroup(NodeSubGroup& subGroup)
{
	int deltaCount = subGroup.index + subGroup.count -
		subGroup.group->GetNumNodes();
	bool grow = false;

	if (deltaCount > 0)
	{
		// Add new nodes to the right
		AddNodesToGroup(subGroup.group, deltaCount);
		grow = true;
	}

	if (subGroup.index < 0)
	{
		// Add new nodes to the left
		AddNodesToLeftOfGroup(
			subGroup.group, -subGroup.index);
		subGroup.index = 0;
		grow = true;
	}

	return grow;
}

Node* RoadNetwork::AddNodeToGroup(NodeGroup* group)
{
	Node* node = new Node();
	node->m_width = m_metrics.laneWidth;
	group->m_nodes.push_back(node);
	return node;
}

void RoadNetwork::AddNodesToGroup(NodeGroup* group, int count)
{
	for (int i = 0; i < count; i++)
	{
		Node* node = new Node();
		node->m_width = m_metrics.laneWidth;
		node->m_index = (int) group->m_nodes.size();
		node->m_nodeGroup = group;

		group->m_nodes.push_back(node);
	}
}

void RoadNetwork::AddNodesToLeftOfGroup(NodeGroup* group, int count)
{
	// Shift sub-group start indexes
	for (int k = 0; k < 2; k++)
	{
		for (unsigned int i = 0; i < group->m_connections[k].size(); i++)
		{
			NodeGroupConnection* connection = group->m_connections[k][i];
			NodeSubGroup& subGroup = connection->m_groups[1 - k];
			subGroup.index += count;
		}
	}

	for (int i = 0; i < count; i++)
	{
		Node* node = new Node();
		node->m_width = m_metrics.laneWidth;
		node->m_index = (int) group->m_nodes.size();
		node->m_nodeGroup = group;

		group->m_nodes.insert(group->m_nodes.begin(), node);

		// Shift the node group's position
		group->m_position.xy += group->GetLeftDirection() * node->m_width;
	}
}

void RoadNetwork::RemoveNodeFromGroup(NodeGroup* group, int count)
{
	// Check if this deletes the entire group
	if (count >= (int) group->m_nodes.size())
	{
		DeleteNodeGroup(group);
		return;
	}

	// Adjust or remove group connections involving this node
	int end = group->m_nodes.size() - count;
	for (int k = 0; k < 2; k++)
	{
		for (unsigned int i = 0; i < group->m_connections[k].size(); i++)
		{
			NodeGroupConnection* connection = group->m_connections[k][i];
			NodeSubGroup& subGroup = connection->m_groups[1 - k];

			int delta = subGroup.index + subGroup.count - end;
			if (delta > 0)
			{
				if (delta >= subGroup.count)
				{
					// Delete this connection entirely
					DeleteNodeGroupConnection(connection);
					i--;
				}
				else
				{
					subGroup.count -= count;
				}
			}
		}
	}

	// Delete the individual nodes from the group
	for (int i = 0; i < count; i++)
	{
		Node* node = group->m_nodes.back();
		group->m_nodes.pop_back();
		delete node;
	}
}

NodeGroupConnection* RoadNetwork::ConnectNodeGroups(NodeGroup* from, NodeGroup* to)
{
	return ConnectNodeSubGroups(
		NodeSubGroup(from, 0, from->GetNumNodes()),
		NodeSubGroup(to, 0, to->GetNumNodes()));
}

NodeGroupConnection* RoadNetwork::ConnectNodeSubGroups(
	NodeGroup* from, int fromIndex, int fromCount,
	NodeGroup* to, int toIndex, int toCount)
{
	return ConnectNodeSubGroups(
		NodeSubGroup(from, fromIndex, fromCount),
		NodeSubGroup(to, toIndex, toCount));
}

NodeGroupConnection* RoadNetwork::ConnectNodeSubGroups(
	const NodeSubGroup& from, const NodeSubGroup& to)
{
	// Check if there is an existing node group connection which can be
	// combined with this one
	for (NodeGroupConnection* connection : from.group->GetOutputs())
	{
		if (connection->m_output.group == to.group &&
			NodeSubGroup::GetOverlap(connection->m_input, from) >= 0 &&
			NodeSubGroup::GetOverlap(connection->m_output, to) >= 0)
		{
			int end0 = Math::Max(from.index + from.count,
				connection->m_input.index + connection->m_input.count);
			int end1 = Math::Max(to.index + to.count,
				connection->m_output.index + connection->m_output.count);
			connection->m_input.index = Math::Min(
				connection->m_input.index, from.index);
			connection->m_output.index = Math::Min(
				connection->m_output.index, to.index);
			connection->m_input.count = end0 - connection->m_input.index;
			connection->m_output.count = end1 - connection->m_output.index;
			return connection;
		}
	}

	// Construct the node group connection
	NodeGroupConnection* connection = new NodeGroupConnection();
	connection->m_id = m_nodeGroupConnectionIdCounter;
	connection->m_input = from;
	connection->m_output = to;
	connection->m_metrics = &m_metrics;
	m_nodeGroupConnections.insert(connection);
	m_nodeGroupConnectionIdCounter++;

	from.group->InsertOutput(connection);
	to.group->InsertInput(connection);

	return connection;
}

NodeGroupTie* RoadNetwork::TieNodeGroups(NodeGroup* a, NodeGroup* b)
{
	// Untie any previous ties
	if (a->m_tie != nullptr || b->m_tie != nullptr)
	{
		if (a->m_tie == b->m_tie)
			return a->m_tie;
		if (a->m_tie != nullptr)
			UntieNodeGroup(a);
		if (b->m_tie != nullptr)
			UntieNodeGroup(b);
	}

	// Construct the tie
	NodeGroupTie* tie = new NodeGroupTie();
	tie->m_position = b->m_position;
	tie->m_direction = b->m_direction;
	tie->m_nodeGroup = b;
	m_nodeGroupTies.insert(tie);

	// Link the two node groups to the tie
	a->m_tie = tie;
	a->m_twin = b;
	b->m_tie = tie;
	b->m_twin = a;
	return tie;
}

void RoadNetwork::UntieNodeGroup(NodeGroup* nodeGroup)
{
	NodeGroupTie* tie = nodeGroup->m_tie;
	nodeGroup->m_twin->m_tie = nullptr;
	nodeGroup->m_twin->m_twin = nullptr;
	nodeGroup->m_tie = nullptr;
	nodeGroup->m_twin = nullptr;
	m_nodeGroupTies.erase(tie);
	delete tie;
}

void RoadNetwork::DeleteNodeGroup(NodeGroup* nodeGroup)
{
	// Delete any connections to the node group
	while (!nodeGroup->m_inputs.empty())
		DeleteNodeGroupConnection(nodeGroup->m_inputs.back());
	while (!nodeGroup->m_outputs.empty())
		DeleteNodeGroupConnection(nodeGroup->m_outputs.back());

	// Delete the node group itself
	m_nodeGroups.erase(nodeGroup);
	delete nodeGroup;
}

void RoadNetwork::DeleteNodeGroupConnection(NodeGroupConnection* connection)
{
	// Delete individiual node connections
	NodeGroup* input = connection->m_input.group;
	NodeGroup* output = connection->m_output.group;
	for (int i = connection->m_input.index; i < connection->m_input.count; i++)
	{
		Node* a = connection->m_input.group->GetNode(i);
		for (int j = connection->m_output.index;
			j < connection->m_output.count; j++)
		{
			Node* b = connection->m_output.group->GetNode(j);
			//Disconnect(a, b);
		}
	}

	input->RemoveOutput(connection);
	output->RemoveInput(connection);

	// Delete the node group connection itself
	m_nodeGroupConnections.erase(connection);
	delete connection;
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

const RoadMetrics& RoadNetwork::GetMetrics() const
{
	return m_metrics;
}

Set<NodeGroup*>& RoadNetwork::GetNodeGroups()
{
	return m_nodeGroups;
}

Set<NodeGroupTie*>& RoadNetwork::GetNodeGroupTies()
{
	return m_nodeGroupTies;
}

Set<NodeGroupConnection*>& RoadNetwork::GetNodeGroupConnections()
{
	return m_nodeGroupConnections;
}

Set<RoadIntersection*>& RoadNetwork::GetIntersections()
{
	return m_intersections;
}

Node* RoadNetwork::CreateNode()
{
	Node* node = new Node();
	node->m_metrics = &m_metrics;
	node->m_width = m_metrics.laneWidth;
	node->m_nodeId = m_nodeIdCounter;
	m_nodeIdCounter++;
	return node;
}

void RoadNetwork::UpdateNodeGeometry()
{
	for (NodeGroupTie* tie : m_nodeGroupTies)
		tie->UpdateGeometry();
	for (NodeGroup* group : m_nodeGroups)
		group->UpdateGeometry();
	for (NodeGroupConnection* surface : m_nodeGroupConnections)
		surface->UpdateGeometry();
	for (NodeGroup* group : m_nodeGroups)
		group->UpdateIntersectionGeometry();
	for (RoadIntersection* intersection : m_intersections)
		intersection->UpdateGeometry();
}
