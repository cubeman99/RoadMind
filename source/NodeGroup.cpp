#include "NodeGroup.h"
#include "NodeGroupConnection.h"

float NodeSubGroup::GetWidth() const
{
	float width = 0.0f;
	for (int i = 0; i < count; i++)
		width += group->GetNode(index + i)->GetWidth();
	return width;
}

Vector2f NodeSubGroup::GetLeftPosition() const
{
	return group->GetNode(index)->GetPosition();
}

Vector2f NodeSubGroup::GetCenterPosition() const
{
	Vector2f leftEdge = group->GetNode(index)->GetPosition();
	float width = GetWidth();
	Vector2f right = group->GetDirection();
	right = Vector2f(-right.y, right.x);
	return (leftEdge + (right * width * 0.5f));
}


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroup::NodeGroup()
	: m_metrics(nullptr)
	, m_position(Vector2f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_tie(nullptr)
	, m_allowPassing(false)
{
}

NodeGroup::~NodeGroup()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

const Vector2f& NodeGroup::GetPosition() const
{
	return m_position;
}

const Vector2f& NodeGroup::GetDirection() const
{
	return m_direction;
}

Vector2f NodeGroup::GetLeftDirection() const
{
	return Vector2f(m_direction.y, -m_direction.x);
}

Vector2f NodeGroup::GetRightDirection() const
{
	return Vector2f(-m_direction.y, m_direction.x);
}

const RoadMetrics* NodeGroup::GetMetrics() const
{
	return m_metrics;
}

NodeGroup* NodeGroup::GetTwin() const
{
	return m_twin;
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

int NodeGroup::GetNumNodes() const
{
	return (int) m_nodes.size();
}

float NodeGroup::GetWidth() const
{
	float width = 0.0f;
	for (unsigned int i = 0; i < m_nodes.size(); i++)
		width += m_nodes[i]->m_width;
	return width;
}

Vector2f NodeGroup::GetCenterPosition() const
{
	float width = GetWidth();
	Vector2f right(-m_direction.y, m_direction.x);
	return (m_position + (right * width * 0.5f));
}

Vector2f NodeGroup::GetRightPosition() const
{
	float width = GetWidth();
	Vector2f right(-m_direction.y, m_direction.x);
	return (m_position + (right * width));
}

Array<NodeGroupConnection*>& NodeGroup::GetInputs()
{
	return m_inputs;
}

Array<NodeGroupConnection*>& NodeGroup::GetOutputs()
{
	return m_outputs;
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void NodeGroup::SetPosition(const Vector2f& position)
{
	m_position = position;
}

void NodeGroup::SetDirection(const Vector2f& direction)
{
	m_direction = direction;
}


//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------

void NodeGroup::UpdateGeometry()
{
	// Adjust the node positions relative to the node group's center
	Vector2f nodePosition = m_position;
	Vector2f right(-m_direction.y, m_direction.x);

	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		Node* node = m_nodes[i];
		node->m_position = nodePosition;
		node->m_endNormal = m_direction;
		node->m_leftTangent = node->m_endNormal;
		node->m_rightTangent = node->m_endNormal;
		nodePosition += node->m_width * right;
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

	for (unsigned int i = 0; i < connections.size(); i++)
	{
		if (connection->m_groups[opposite].index <=
			connections[i]->m_groups[opposite].index &&
			connection->m_groups[opposite].count <=
			connections[i]->m_groups[opposite].count)
		{
			connections.insert(connections.begin() + i, connection);
			return;
		}
	}

	connections.push_back(connection);
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
