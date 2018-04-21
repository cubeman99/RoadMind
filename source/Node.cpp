#include "Node.h"
#include "Connection.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Node::Node()
	: m_position(Vector2f::ZERO)
	, m_endNormal(Vector2f::UNITX)
	, m_leftTangent(Vector2f::UNITX)
	, m_rightTangent(Vector2f::UNITX)
	, m_width(1.0f)
	, m_leftNode(nullptr)
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

Vector2f Node::GetEndNormal() const
{
	return m_endNormal;
}

Vector2f Node::GetEndTangent() const
{
	return Vector2f(-m_endNormal.y, m_endNormal.x);
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
	return m_leftTangent;
}

Vector2f Node::GetRightEdgeTangent() const
{
	return m_rightTangent;
}

Vector2f Node::GetCenter() const
{
	return (m_position + (GetEndTangent() * m_width * 0.5f));
}

Node* Node::GetLeftNode() const
{
	return m_leftNode;
}

Node* Node::GetRightNode() const
{
	return m_rightNode;
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
	if (m_leftNode != nullptr)
		return m_leftDivider;
	else
		return LaneDivider::SOLID;
}

LaneDivider Node::GetRightLaneDivider() const
{
	if (m_rightNode != nullptr)
		return m_rightNode->m_leftDivider;
	else
		return LaneDivider::SOLID;
}

bool Node::IsLeftMostLane() const
{
	return (m_leftNode == nullptr || m_leftNode->m_leftNode == this);
}

bool Node::IsRightMostLane() const
{
	return (m_rightNode == nullptr);
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void Node::SetLeftNode(Node* node)
{
	m_leftNode = node;
}

void Node::SetRightNode(Node* node)
{
	m_rightNode = node;
}


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
	m_endNormal = normal;
	m_leftTangent = normal;
	m_rightTangent = normal;
}

void Node::SetLeftEdgeTangent(const Vector2f& tangent)
{
	m_leftTangent = tangent;
}

void Node::SetRightEdgeTangent(const Vector2f& tangent)
{
	m_rightTangent = tangent;
}


//-----------------------------------------------------------------------------
// Internal Methods
//-----------------------------------------------------------------------------

void Node::UpdateGeometry()
{
	m_leftDivider = LaneDivider::SOLID;
	if (m_rightNode == nullptr && m_leftNode != nullptr &&
		m_leftNode->m_leftNode == this && m_leftNode->m_rightNode == nullptr)
		m_leftDivider = LaneDivider::DASHED;
	else if (m_leftNode != nullptr && m_leftNode->m_rightNode == this)
		m_leftDivider = LaneDivider::DASHED;

	if (m_leftNode != nullptr)
	{
		if (m_leftNode->m_leftNode == this)
		{
			if (m_nodeId > m_leftNode->m_nodeId)
			{
				m_position = m_leftNode->m_position;
				m_endNormal = -m_leftNode->m_endNormal;
			}
		}
		else
		{
			m_position = m_leftNode->GetRightEdge();
			m_endNormal = m_leftNode->m_endNormal;
		}

		m_leftTangent = m_endNormal;
		m_rightTangent = m_endNormal;
	}
}
