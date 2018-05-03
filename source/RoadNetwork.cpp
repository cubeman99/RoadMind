#include "RoadNetwork.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

RoadNetwork::RoadNetwork()
{
	m_nodeIdCounter = 1;

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
	for (NodeGroupConnection* surface : m_nodeGroupConnections)
		delete surface;
	m_nodeGroupConnections.clear();
	for (NodeGroupTie* tie : m_nodeGroupTies)
		delete tie;
	m_nodeGroupTies.clear();
	for (Connection* connection : m_connections)
		delete connection;
	m_connections.clear();
	for (NodeGroup* nodeGroup : m_nodeGroups)
		delete nodeGroup;
	m_nodeGroups.clear();
	for (Node* node : m_nodes)
		delete node;
	m_nodes.clear();
}

NodeGroup* RoadNetwork::CreateNodeGroup(const Vector2f& position,
	const Vector2f& direction, int laneCount)
{
	// Construct the node group
	NodeGroup* group = new NodeGroup();
	group->m_metrics = &m_metrics;
	group->m_position = position;
	group->m_direction = direction;
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
		m_nodes.insert(node);
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
		m_nodes.insert(node);

		// Shift the node group's position
		group->m_position += group->GetLeftDirection() * node->m_width;
	}
}

void RoadNetwork::RemoveNodeFromGroup(NodeGroup* group, int count)
{
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
		while (!node->m_inputs.empty())
			DeleteConnection(*node->m_inputs.begin());
		while (!node->m_outputs.empty())
			DeleteConnection(*node->m_outputs.begin());

		m_nodes.erase(node);
		group->m_nodes.pop_back();
		delete node;
	}

	if (group->m_nodes.empty())
	{
	}
	else
	{
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
	// Check if there is an existing node group connection can be
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
			connection->m_input.index = end0 - connection->m_input.index;
			connection->m_output.index = end1 - connection->m_output.index;
			return connection;
		}
	}

	// Construct the node group connection
	NodeGroupConnection* connection = new NodeGroupConnection();
	connection->m_input = from;
	connection->m_output = to;
	connection->m_twin = nullptr;
	connection->m_metrics = &m_metrics;
	m_nodeGroupConnections.insert(connection);

	from.group->InsertOutput(connection);
	to.group->InsertInput(connection);

	// Connect the individual nodes
	//Node* nodes[2];
	//int maxCount = Math::Max(from.count, to.count);
	//for (int i = 0; i < maxCount; i++)
	//{
		//nodes[0] = from.group->GetNode(
			//from.index + Math::Min(from.count - 1, i));
		//nodes[1] = to.group->GetNode(
			//to.index + Math::Min(to.count - 1, i));
		//Connect(nodes[0], nodes[1]);
	//}

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
	nodeGroup->m_tie = nullptr;
	nodeGroup->m_twin->m_tie = nullptr;
	m_nodeGroupTies.erase(tie);
	delete tie;
}

void RoadNetwork::DeleteNodeGroup(NodeGroup* nodeGroup)
{
	// TODO: Delete any connections to the node group

	// Delete all nodes in the node group
	for (unsigned int i = 0; i < nodeGroup->m_nodes.size(); i++)
	{
		Node* node = nodeGroup->m_nodes[i];
		delete node;
		m_nodes.erase(node);
	}

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

std::set<Node*>& RoadNetwork::GetNodes()
{
	return m_nodes;
}

std::set<Connection*>& RoadNetwork::GetConnections()
{
	return m_connections;
}

std::set<NodeGroup*>& RoadNetwork::GetNodeGroups()
{
	return m_nodeGroups;
}

std::set<NodeGroupTie*>& RoadNetwork::GetNodeGroupTies()
{
	return m_nodeGroupTies;
}

std::set<NodeGroupConnection*>& RoadNetwork::GetNodeGroupConnections()
{
	return m_nodeGroupConnections;
}

Node* RoadNetwork::CreateNode()
{
	Node* node = new Node();
	node->m_metrics = &m_metrics;
	node->m_width = m_metrics.laneWidth;
	m_nodes.insert(node);
	node->m_nodeId = m_nodeIdCounter;
	m_nodeIdCounter++;
	return node;
}

Node* RoadNetwork::CreateNode(const Vector2f& position, const Vector2f& direction, float width)
{
	Node* node = CreateNode();
	node->m_position = position;
	node->m_endNormal = Vector2f::Normalize(direction);
	node->m_leftTangent = node->m_endNormal;
	node->m_rightTangent = node->m_endNormal;
	node->m_width = width;
	return node;
}

void RoadNetwork::DeleteNode(Node* node)
{
	// TODO: unsew left/right

	// Disconnect all inputs and outputs
	for (Connection* connection : node->GetInputs())
	{
		connection->GetInput()->m_outputs.erase(connection);
		m_connections.erase(connection);
		delete connection;
	}
	for (Connection* connection : node->GetOutputs())
	{
		connection->GetOutput()->m_inputs.erase(connection);
		m_connections.erase(connection);
		delete connection;
	}

	m_nodes.erase(node);
	delete node;
}

Connection* RoadNetwork::Connect(Node* from, Node* to)
{
	Connection* connection = from->GetOutputConnection(to);
	if (connection == nullptr)
	{
		connection = new Connection(from, to);
		m_connections.insert(connection);
		from->m_outputs.insert(connection);
		to->m_inputs.insert(connection);
	}
	return connection;
}

void RoadNetwork::Disconnect(Node* from, Node* to)
{
	Connection* connection = from->GetOutputConnection(to);
	if (connection != nullptr)
		DeleteConnection(connection);
}

void RoadNetwork::DeleteConnection(Connection* connection)
{
	m_connections.erase(connection);
	connection->m_input->m_outputs.erase(connection);
	connection->m_output->m_inputs.erase(connection);
	delete connection;
}

void RoadNetwork::UpdateNodeGeometry()
{
	for (NodeGroupTie* tie : m_nodeGroupTies)
		tie->UpdateGeometry();
	for (NodeGroup* group : m_nodeGroups)
		group->UpdateGeometry();
	for (NodeGroupConnection* surface : m_nodeGroupConnections)
		surface->UpdateGeometry();
	for (Node* node : m_nodes)
		node->UpdateGeometry();
	for (Connection* connection : m_connections)
		connection->CalcVertices();
}
