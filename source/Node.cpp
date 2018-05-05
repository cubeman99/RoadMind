#include "Node.h"
#include "Connection.h"
#include "NodeGroup.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Node::Node()
	: m_position(Vector2f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_width(1.0f)
	, m_leftDivider(LaneDivider::DASHED)
	, m_nodeGroup(nullptr)
	, m_index(0)
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

int Node::GetIndex() const
{
	return m_index;
}

NodeGroup* Node::GetNodeGroup()
{
	return m_nodeGroup;
}

const RoadMetrics* Node::GetMetrics() const
{
	return m_metrics;
}

float Node::GetWidth() const
{
	return m_width;
}

unsigned int Node::GetNodeId() const
{
	return m_nodeId;
}

Vector2f Node::GetDirection() const
{
	return m_direction;
}

Vector2f Node::GetEndTangent() const
{
	return Vector2f(-m_direction.y, m_direction.x);
}

Vector2f Node::GetPosition() const
{
	return m_position;
}

Vector2f Node::GetLeftEdge() const
{
	return m_position;
}

Vector2f Node::GetRightEdge() const
{
	return (m_position + (GetEndTangent() * m_width));
}

Vector2f Node::GetLeftEdgeTangent() const
{
	return m_direction;
}

Vector2f Node::GetRightEdgeTangent() const
{
	return m_direction;
}

Vector2f Node::GetCenter() const
{
	return (m_position + (GetEndTangent() * m_width * 0.5f));
}

Node* Node::GetLeftNode() const
{
	if (m_index <= 0)
		return nullptr;
	else
		return m_nodeGroup->GetNode(m_index - 1);
}

Node* Node::GetRightNode() const
{
	if (m_index >= m_nodeGroup->GetNumNodes() - 1)
		return nullptr;
	else
		return m_nodeGroup->GetNode(m_index + 1);
}

int Node::GetNumInputs() const
{
	return m_inputs.size();
}

int Node::GetNumOutputs() const
{
	return m_outputs.size();
}

int Node::GetNumConnections(InputOutput type) const
{
	if (type == InputOutput::INPUT)
		return m_inputs.size();
	else if (type == InputOutput::OUTPUT)
		return m_outputs.size();
	else
		return (m_inputs.size() + m_outputs.size());
}

std::set<Connection*>& Node::GetConnections(InputOutput type)
{
	if (type == InputOutput::INPUT)
		return m_inputs;
	else
		return m_outputs;
}

std::set<Connection*>& Node::GetInputs()
{
	return m_inputs;
}

std::set<Connection*>& Node::GetOutputs()
{
	return m_outputs;
}

bool Node::HasInput(Node* node) const
{
	return (GetInputConnection(node) != nullptr);
}

bool Node::HasOutput(Node* node) const
{
	return (GetOutputConnection(node) != nullptr);
}

Connection* Node::GetInputConnection(Node* node) const
{
	for (Connection* connection : m_inputs)
	{
		if (connection->GetInput() == node)
			return connection;
	}
	return nullptr;
}

Connection* Node::GetOutputConnection(Node* node) const
{
	for (Connection* connection : m_outputs)
	{
		if (connection->GetOutput() == node)
			return connection;
	}
	return nullptr;
}

LaneDivider Node::GetLeftLaneDivider() const
{
	return LaneDivider::SOLID;
}

LaneDivider Node::GetRightLaneDivider() const
{
	return LaneDivider::SOLID;
}

bool Node::IsLeftMostLane() const
{
	return (m_index == 0);
}

bool Node::IsRightMostLane() const
{
	return (m_index == m_nodeGroup->GetNumNodes() - 1);
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void Node::SetWidth(float width)
{
	m_width = width;
}

void Node::SetLeftPosition(const Vector2f& position)
{
	m_position = position;
}

void Node::SetCenterPosition(const Vector2f& center)
{
	m_position = center - (GetEndTangent() * m_width * 0.5f);
}

void Node::SetEndNormal(const Vector2f& normal)
{
	m_direction = normal;
}


//-----------------------------------------------------------------------------
// Internal Methods
//-----------------------------------------------------------------------------

void Node::UpdateGeometry()
{
	//if (m_leftNode != nullptr)
	//{
	//	if (m_leftNode->m_leftNode == this)
	//	{
	//		if (m_nodeId > m_leftNode->m_nodeId)
	//		{
	//			m_position = m_leftNode->m_position;
	//			m_direction = -m_leftNode->m_direction;
	//		}
	//	}
	//	else
	//	{
	//		m_position = m_leftNode->GetRightEdge();
	//		m_direction = m_leftNode->m_direction;
	//	}
	//}
}
