#include "RoadNetwork.h"


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

void RoadNetwork::ClearNodes()
{
	for (RoadSurface* surface : m_roadSurfaces)
		delete surface;
	m_roadSurfaces.clear();
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

NodeGroup* RoadNetwork::CreateNodeGroup(const Vector2f& position, const Vector2f& direction, int laneCount)
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
		prev->m_rightNode = node;
		node->m_leftNode = prev;
		prev = node;
		group->m_nodes.push_back(node);
	}

	return group;
}

RoadSurface* RoadNetwork::ConnectNodeGroups(NodeGroup* from, NodeGroup* to)
{
	return ConnectNodeSubGroups(from, 0, from->GetNumNodes(),
		to, 0, to->GetNumNodes());
}

RoadSurface* RoadNetwork::ConnectNodeSubGroups(
	NodeGroup* from, int fromIndex, int fromCount,
	NodeGroup* to, int toIndex, int toCount)
{
	// Construct the node group connection
	RoadSurface* surface = new RoadSurface();
	surface->m_groups[0] = from;
	surface->m_indexes[0] = fromIndex;
	surface->m_counts[0] = fromCount;
	surface->m_groups[1] = to;
	surface->m_indexes[1] = toIndex;
	surface->m_counts[1] = toCount;
	surface->m_twin = nullptr;
	surface->m_metrics = &m_metrics;
	m_roadSurfaces.insert(surface);

	// Connect the individual nodes
	Node* nodes[2];
	int maxCount = Math::Max(fromCount, toCount);
	for (int i = 0; i < maxCount; i++)
	{
		nodes[0] = from->GetNode(fromIndex + Math::Min(fromCount - 1, i));
		nodes[1] = to->GetNode(toIndex + Math::Min(toCount - 1, i));
		Connect(nodes[0], nodes[1]);
	}

	return surface;
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

void RoadNetwork::DeleteNodeGroupConnection(RoadSurface* connection)
{
	// Delete individiual node connections
	NodeGroup* input = connection->GetInput();
	for (int i = connection->m_indexes[0]; i < connection->m_counts[0]; i++)
	{
		Node* a = connection->GetInput()->GetNode(i);
		for (int j = connection->m_indexes[1]; j < connection->m_counts[1]; j++)
		{
			Node* b = connection->GetOutput()->GetNode(j);
			Disconnect(a, b);
		}
	}

	// Delete the node group connection itself
	m_roadSurfaces.erase(connection);
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

std::set<RoadSurface*>& RoadNetwork::GetRoadSurfaces()
{
	return m_roadSurfaces;
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

	if (node->m_leftNode != nullptr)
	{
		if (node->m_leftNode->m_leftNode == node)
			node->m_leftNode->m_leftNode = nullptr;
		else if (node->m_leftNode->m_rightNode == node)
			node->m_leftNode->m_rightNode = nullptr;
	}
	if (node->m_rightNode != nullptr)
	{
		if (node->m_rightNode->m_leftNode == node)
			node->m_rightNode->m_leftNode = nullptr;
		else if (node->m_rightNode->m_rightNode == node)
			node->m_rightNode->m_rightNode = nullptr;
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
	{
		m_connections.erase(connection);
		delete connection;
		from->m_outputs.erase(connection);
		to->m_inputs.erase(connection);
	}
}

void RoadNetwork::SewNode(Node* node, Node* left)
{
	node->m_leftNode = left;
	left->m_rightNode = node;
}

void RoadNetwork::SewOpposite(Node* node, Node* left)
{
	node->m_leftNode = left;
	left->m_leftNode = node;
}

void RoadNetwork::UpdateNodeGeometry()
{
	for (NodeGroupTie* tie : m_nodeGroupTies)
		tie->UpdateGeometry();
	for (NodeGroup* group : m_nodeGroups)
		group->UpdateGeometry();
	for (RoadSurface* surface : m_roadSurfaces)
		surface->UpdateGeometry();
	//for (Node* node : m_nodes)
	//node->UpdateGeometry();
	for (Connection* connection : m_connections)
		connection->CalcVertices();
}
