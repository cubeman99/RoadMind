#include "RoadNetwork.h"
#include <map>
#include <algorithm>


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

RoadNetwork::RoadNetwork()
{
	m_nodeGroupConnectionIdCounter = 1;
	m_intersectionIdCounter = 1;
	m_nodeGroupIdCounter = 1;
	m_tieIdCounter = 1;

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
	m_nodeGroupConnectionIdCounter = 1;
	m_intersectionIdCounter = 1;
	m_nodeGroupIdCounter = 1;
	m_tieIdCounter = 1;

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
	group->m_id = m_nodeGroupIdCounter++;
	group->m_metrics = &m_metrics;
	group->m_position = position;
	group->m_direction = direction;
	group->m_leftShoulderWidth = m_metrics.laneWidth * 0.25f;
	group->m_rightShoulderWidth = m_metrics.laneWidth * 0.25f;
	m_nodeGroups.insert(group);

	// Create the left-most node
	Node* node = new Node();
	node->m_nodeGroup = group;
	node->m_width = m_metrics.laneWidth;
	node->m_index = 0;
	group->m_nodes.push_back(node);

	// Add nodes to the right
	Node* prev = node;
	for (int i = 1; i < laneCount; i++)
	{
		node = new Node();
		node->m_index = i;
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
	intersection->m_id = m_intersectionIdCounter++;
	intersection->Construct(nodeGroups);
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
	node->m_index = (int) group->m_nodes.size();
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
		if (connection->GetOutput().group == to.group &&
			NodeSubGroup::GetOverlap(connection->GetInput(), from) >= 0 &&
			NodeSubGroup::GetOverlap(connection->GetOutput(), to) >= 0)
		{
			int end0 = Math::Max(from.index + from.count,
				connection->GetInput().index + connection->GetInput().count);
			int end1 = Math::Max(to.index + to.count,
				connection->GetOutput().index + connection->GetOutput().count);
			connection->GetInput().index = Math::Min(
				connection->GetInput().index, from.index);
			connection->GetOutput().index = Math::Min(
				connection->GetOutput().index, to.index);
			connection->GetInput().count = end0 - connection->GetInput().index;
			connection->GetOutput().count = end1 - connection->GetOutput().index;
			return connection;
		}
	}

	// Construct the node group connection
	NodeGroupConnection* connection = new NodeGroupConnection();
	connection->m_id = m_nodeGroupConnectionIdCounter++;
	connection->SetInput(from);
	connection->SetOutput(to);
	connection->m_metrics = &m_metrics;
	m_nodeGroupConnections.insert(connection);

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
	tie->m_id = m_tieIdCounter++;
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
	// Untie the node group
	if (nodeGroup->IsTied())
		UntieNodeGroup(nodeGroup);

	// Delete any connections to the node group
	while (!nodeGroup->GetInputs().empty())
		DeleteNodeGroupConnection(nodeGroup->GetInputs().back());
	while (!nodeGroup->GetOutputs().empty())
		DeleteNodeGroupConnection(nodeGroup->GetOutputs().back());

	// Notify its intersection that it was deleted
	if (nodeGroup->GetIntersection() != nullptr)
		RemoveNodeGroupFromIntersection(nodeGroup);

	// Delete the node group itself
	m_nodeGroups.erase(nodeGroup);
	delete nodeGroup;
}

void RoadNetwork::RemoveNodeGroupFromIntersection(NodeGroup* nodeGroup)
{
	RoadIntersection* intersection = nodeGroup->GetIntersection();
	if (intersection->GetPoints().size() == 2)
	{
		// Intersection is too small, delete it
		DeleteIntersection(intersection);
	}
	else
	{
		// Recreate the intersection without this node group
		Set<NodeGroup*> groups;
		for (RoadIntersectionPoint* point : intersection->GetPoints())
		{
			if (point->GetNodeGroup() != nodeGroup)
				groups.insert(point->GetNodeGroup());
		}
		intersection->Construct(groups);
		nodeGroup->m_intersection = nullptr;
	}
}

void RoadNetwork::DeleteIntersection(RoadIntersection* intersection)
{
	// Disconnect node groups from the intersection
	for (RoadIntersectionPoint* point : intersection->GetPoints())
		point->GetNodeGroup()->m_intersection = nullptr;

	// Delete the intersection itself
	m_intersections.erase(intersection);
	delete intersection;
}

void RoadNetwork::DeleteNodeGroupConnection(NodeGroupConnection* connection)
{
	// Delete individiual node connections
	NodeGroup* input = connection->GetInput().group;
	NodeGroup* output = connection->GetOutput().group;
	for (int i = connection->GetInput().index; i < connection->GetInput().count; i++)
	{
		Node* a = connection->GetInput().group->GetNode(i);
		for (int j = connection->GetOutput().index;
			j < connection->GetOutput().count; j++)
		{
			Node* b = connection->GetOutput().group->GetNode(j);
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

void RoadNetwork::UpdateNodeGeometry()
{
	for (NodeGroupTie* tie : m_nodeGroupTies)
		tie->UpdateGeometry();
	for (NodeGroup* group : m_nodeGroups)
		group->UpdateGeometry();
	for (NodeGroupConnection* connection : m_nodeGroupConnections)
		connection->UpdateGeometry();
	for (NodeGroup* group : m_nodeGroups)
		group->UpdateIntersectionGeometry();
	for (RoadIntersection* intersection : m_intersections)
		intersection->UpdateGeometry();
}

void RoadNetwork::Simulate(Seconds dt)
{
	for (RoadIntersection* intersection : m_intersections)
		intersection->Update(dt);
}


//-----------------------------------------------------------------------------
// Save & Load
//-----------------------------------------------------------------------------

bool RoadNetwork::Save(const Path& path)
{
	File file(path);
	if (file.Open(FileAccess::WRITE, FileType::BINARY).Failed())
		return false;

	unsigned int count;

	// Save node groups
	count = m_nodeGroups.size();
	file.Write(&m_nodeGroupIdCounter, sizeof(int));
	file.Write(&count, sizeof(unsigned int));
	for (NodeGroup* group : m_nodeGroups)
	{
		file.Write(&group->m_id, sizeof(int));
		file.Write(&group->m_position, sizeof(Vector3f));
		file.Write(&group->m_direction, sizeof(Vector2f));
		file.Write(&group->m_leftShoulderWidth, sizeof(Meters));
		file.Write(&group->m_rightShoulderWidth, sizeof(Meters));
		file.Write(&group->m_allowPassing, sizeof(Meters));
		SavePointer(file, group->m_twin);
		SavePointer(file, group->m_tie);
		SavePointer(file, group->m_intersection);

		// Save individual nodes
		count = group->m_nodes.size();
		file.Write(&count, sizeof(unsigned int));
		for (unsigned int i = 0; i < group->m_nodes.size(); i++)
		{
			Node* node = group->m_nodes[i];
			file.Write(&node->m_width, sizeof(Meters));
			file.Write(&node->m_position, sizeof(Vector3f));
			file.Write(&node->m_direction, sizeof(Vector2f));
			file.Write(&node->m_leftDivider, sizeof(node->m_leftDivider));
			file.Write(&node->m_index, sizeof(node->m_index));
		}

		// Save input & output connections
		for (unsigned int i = 0; i < 2; i++)
		{
			count = group->m_connections[i].size();
			file.Write(&count, sizeof(unsigned int));
			for (unsigned int j = 0; j < group->m_connections[i].size(); j++)
				SavePointer(file, group->m_connections[i][j]);
		}
	}

	// Save node group ties
	file.Write(&m_tieIdCounter, sizeof(int));
	count = m_nodeGroupTies.size();
	file.Write(&count, sizeof(unsigned int));
	for (NodeGroupTie* tie : m_nodeGroupTies)
	{
		file.Write(&tie->m_id, sizeof(int));
		file.Write(&tie->m_position, sizeof(Vector3f));
		file.Write(&tie->m_direction, sizeof(Vector2f));
		file.Write(&tie->m_centerDividerWidth, sizeof(Meters));
		SavePointer(file, tie->m_nodeGroup);
	}

	// Save node group connections
	file.Write(&m_nodeGroupConnectionIdCounter, sizeof(int));
	count = m_nodeGroupConnections.size();
	file.Write(&count, sizeof(unsigned int));
	for (NodeGroupConnection* connection : m_nodeGroupConnections)
	{
		file.Write(&connection->m_id, sizeof(int));
		file.Write(&connection->GetInput().index, sizeof(int));
		file.Write(&connection->GetInput().count, sizeof(int));
		SavePointer(file, connection->GetInput().group);
		file.Write(&connection->GetOutput().index, sizeof(int));
		file.Write(&connection->GetOutput().count, sizeof(int));
		SavePointer(file, connection->GetOutput().group);
	}

	// Save intersections
	file.Write(&m_intersectionIdCounter, sizeof(int));
	count = m_intersections.size();
	file.Write(&count, sizeof(unsigned int));
	for (RoadIntersection* intersection : m_intersections)
	{
		file.Write(&intersection->m_id, sizeof(int));

		// Save points
		count = intersection->m_points.size();
		file.Write(&count, sizeof(unsigned int));
		for (unsigned int i = 0; i < intersection->m_points.size(); i++)
		{
			RoadIntersectionPoint* point = intersection->m_points[i];
			file.Write(&point->m_ioType, sizeof(IOType));
			SavePointer(file, point->m_nodeGroup);
		}

		// Save edges
		count = intersection->m_edges.size();
		file.Write(&count, sizeof(unsigned int));
		for (unsigned int i = 0; i < intersection->m_edges.size(); i++)
		{
			RoadIntersectionEdge* edge = intersection->m_edges[i];
			for (unsigned int j = 0; j < 2; j++)
			{
				auto it = std::find(intersection->m_points.begin(),
					intersection->m_points.end(), edge->m_points[j]);
				unsigned int index = it - intersection->m_points.begin();
				file.Write(&index, sizeof(unsigned int));
			}
		}
	}

	return true;
}

bool RoadNetwork::Load(const Path& path)
{
	ClearNodes();
	File file(path);
	if (file.Open(FileAccess::READ, FileType::BINARY).Failed())
		return false;
	unsigned int count;
	unsigned int count2;

	// Read node groups
	file.Read(&m_nodeGroupIdCounter, sizeof(int));
	file.Read(&count, sizeof(unsigned int));
	for (unsigned int i = 0; i < count; i++)
	{
		NodeGroup* group = LoadPointer(file, m_nodeGroups);
		file.Read(&group->m_position, sizeof(Vector3f));
		file.Read(&group->m_direction, sizeof(Vector2f));
		file.Read(&group->m_leftShoulderWidth, sizeof(Meters));
		file.Read(&group->m_rightShoulderWidth, sizeof(Meters));
		file.Read(&group->m_allowPassing, sizeof(Meters));
		group->m_twin = LoadPointer(file, m_nodeGroups);
		group->m_tie = LoadPointer(file, m_nodeGroupTies);
		group->m_intersection = LoadPointer(file, m_intersections);
		group->m_intersection = nullptr;

		// Read individual nodes
		file.Read(&count2, sizeof(unsigned int));
		group->m_nodes.resize(count2);
		for (unsigned int j = 0; j < count2; j++)
		{
			Node* node = new Node();
			group->m_nodes[j] = node;
			node->m_nodeGroup = group;
			file.Read(&node->m_width, sizeof(Meters));
			file.Read(&node->m_position, sizeof(Vector3f));
			file.Read(&node->m_direction, sizeof(Vector2f));
			file.Read(&node->m_leftDivider, sizeof(node->m_leftDivider));
			file.Read(&node->m_index, sizeof(node->m_leftDivider));
			node->m_index = j;
		}

		// Read input & output connections
		for (unsigned int i = 0; i < 2; i++)
		{
			file.Read(&count2, sizeof(unsigned int));
			group->m_connections[i].resize(count2);
			for (unsigned int j = 0; j < count2; j++)
				group->m_connections[i][j] = LoadPointer(file, m_nodeGroupConnections);
		}
	}


	// Read node group ties
	file.Read(&m_tieIdCounter, sizeof(int));
	file.Read(&count, sizeof(unsigned int));
	for (unsigned int i = 0; i < count; i++)
	{
		NodeGroupTie* tie = LoadPointer(file, m_nodeGroupTies);
		file.Read(&tie->m_position, sizeof(Vector3f));
		file.Read(&tie->m_direction, sizeof(Vector2f));
		file.Read(&tie->m_centerDividerWidth, sizeof(Meters));
		tie->m_nodeGroup = LoadPointer(file, m_nodeGroups);
	}

	// Read node group connections
	file.Read(&m_nodeGroupConnectionIdCounter, sizeof(int));
	file.Read(&count, sizeof(unsigned int));
	for (unsigned int i = 0; i < count; i++)
	{
		NodeGroupConnection* connection =
			LoadPointer(file, m_nodeGroupConnections);
		file.Read(&connection->GetInput().index, sizeof(int));
		file.Read(&connection->GetInput().count, sizeof(int));
		connection->GetInput().group = LoadPointer(file, m_nodeGroups);
		file.Read(&connection->GetOutput().index, sizeof(int));
		file.Read(&connection->GetOutput().count, sizeof(int));
		connection->GetOutput().group = LoadPointer(file, m_nodeGroups);
	}

	// Read intersections
	file.Read(&m_nodeGroupConnectionIdCounter, sizeof(int));
	file.Read(&count, sizeof(unsigned int));
	for (unsigned int i = 0; i < count; i++)
	{
		RoadIntersection* intersection = LoadPointer(file, m_intersections);

		// Read points
		file.Read(&count2, sizeof(unsigned int));
		intersection->m_points.resize(count2);
		for (unsigned int j = 0; j < count2; j++)
		{
			RoadIntersectionPoint* point = new RoadIntersectionPoint();
			intersection->m_points[j] = point;
			file.Read(&point->m_ioType, sizeof(IOType));
			point->m_nodeGroup = LoadPointer(file, m_nodeGroups);
			if (point->m_ioType == IOType::INPUT)
				point->m_nodeGroup->m_intersection = intersection;
			else
				point->m_nodeGroup->m_inputIntersection = intersection;
		}

		// Read edges
		file.Read(&count2, sizeof(unsigned int));
		intersection->m_edges.resize(count2);
		for (unsigned int j = 0; j < count2; j++)
		{
			RoadIntersectionEdge* edge = new RoadIntersectionEdge();
			intersection->m_edges[j] = edge;
			for (unsigned int k = 0; k < 2; k++)
			{
				unsigned int index;
				file.Read(&index, sizeof(unsigned int));
				edge->m_points[k] = intersection->m_points[index];
			}
		}

		intersection->CreateTrafficLightProgram();
	}

	return true;
}

