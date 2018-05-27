#include "NodeGroupTie.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

NodeGroupTie::NodeGroupTie()
	: m_centerDividerWidth(0.0f)
	, m_position(Vector3f::ZERO)
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

const Vector3f& NodeGroupTie::GetPosition() const
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


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void NodeGroupTie::SetPosition(const Vector3f& position)
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
// Geometry
//-----------------------------------------------------------------------------

void NodeGroupTie::UpdateGeometry()
{
	Vector2f normal = RightPerpendicular(m_direction);

	m_nodeGroup->m_position = Vector3f(
		m_position.xy + (normal * m_centerDividerWidth * 0.5f), m_position.z);
	m_nodeGroup->m_direction = m_direction;

	NodeGroup* twin = m_nodeGroup->GetTwin();
	twin->m_position = Vector3f(
		m_position.xy - (normal * m_centerDividerWidth * 0.5f), m_position.z);
	twin->m_direction = -m_direction;
}

