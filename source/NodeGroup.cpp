#include "NodeGroup.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroupTie::NodeGroupTie()
	: m_centerDividerWidth(0.0f)
	, m_position(Vector2f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_nodeGroup(nullptr)
{
}

NodeGroupTie::~NodeGroupTie()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

float NodeGroupTie::GetCenterWidth() const
{
	return m_centerDividerWidth;
}

const Vector2f& NodeGroupTie::GetPosition() const
{
	return m_position;
}

const Vector2f& NodeGroupTie::GetDirection() const
{
	return m_direction;
}

NodeGroup* NodeGroupTie::GetNodeGroup() const
{
	return m_nodeGroup;
}

NodeGroup* NodeGroupTie::GetNodeGroupTwin() const
{
	return m_nodeGroup->GetTwin();
}

void NodeGroupTie::UpdateGeometry()
{
	Vector2f normal = m_direction;
	normal = Vector2f(-normal.y, normal.x);

	m_nodeGroup->SetPosition(m_position + (normal * m_centerDividerWidth * 0.5f));
	m_nodeGroup->SetDirection(m_direction);

	NodeGroup* twin = m_nodeGroup->GetTwin();
	twin->SetPosition(m_position - (normal * m_centerDividerWidth * 0.5f));
	twin->SetDirection(-m_direction);
}

//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void NodeGroupTie::SetPosition(const Vector2f& position)
{
	m_position = position;
}

void NodeGroupTie::SetDirection(const Vector2f& direction)
{
	m_direction = direction;
}

void NodeGroupTie::SetCenterWidth(Meters centerWidth)
{
	m_centerDividerWidth = centerWidth;
}





//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroup::NodeGroup()
	: m_metrics(nullptr)
	, m_position(Vector2f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_tie(nullptr)
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

