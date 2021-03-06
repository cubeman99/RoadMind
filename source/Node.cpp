#include "Node.h"
#include "Connection.h"
#include "NodeGroup.h"
#include "RoadIntersection.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Node::Node()
	: m_position(Vector3f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_width(1.0f)
	, m_leftDivider(LaneDivider::DASHED)
	, m_nodeGroup(nullptr)
	, m_index(0)
	, m_hasStopSign(false)
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

Vector2f Node::GetDirection() const
{
	return m_direction;
}

Vector2f Node::GetEndTangent() const
{
	return RightPerpendicular(m_direction);
}

Vector3f Node::GetPosition() const
{
	return m_position;
}

Vector3f Node::GetLeftEdge() const
{
	return m_position;
}

Vector3f Node::GetRightEdge() const
{
	return Vector3f(m_position.xy + (GetEndTangent() * m_width), m_position.z);
}

Vector2f Node::GetLeftEdgeTangent() const
{
	return m_direction;
}

Vector2f Node::GetRightEdgeTangent() const
{
	return m_direction;
}

Vector3f Node::GetCenter() const
{
	return Vector3f(m_position.xy + (GetEndTangent() * m_width * 0.5f),
		m_position.z);
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


bool Node::HasStopSign() const
{
	return m_hasStopSign;
}

TrafficLightSignal Node::GetSignal() const
{
	if (m_nodeGroup->GetIntersection() != nullptr &&
		m_nodeGroup->GetIntersection()->GetTrafficLightProgram() != nullptr)
	{
		return m_nodeGroup->GetIntersection()->GetTrafficLightProgram()->GetSignal(this);
	}
	return TrafficLightSignal::NONE;
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void Node::SetWidth(float width)
{
	m_width = width;
}

void Node::SetLeftPosition(const Vector3f& position)
{
	m_position = position;
}

void Node::SetCenterPosition(const Vector3f& center)
{
	m_position = Vector3f(
		center.xy - (GetEndTangent() * m_width * 0.5f), center.z);
}

void Node::SetEndNormal(const Vector2f& normal)
{
	m_direction = normal;
}
