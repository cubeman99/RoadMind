#include "NodeGroupTie.h"


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
// Geometry
//-----------------------------------------------------------------------------

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

