#include "Driver.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Driver::Driver()
	: m_velocity(Vector2f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_roadNetwork(nullptr)
	, m_speed(20.0f)
	, m_distance(0.0f)
{
}

Driver::Driver(RoadNetwork* network, Node* node)
	: m_nodeCurrent(node)
	, m_nodeTarget(nullptr)
	, m_connection(nullptr)
	, m_roadNetwork(network)
	, m_speed(20.0f)
	, m_distance(0.0f)
{
	m_speed = Random::NextFloat(10.0f, 20.0f);
	if (node != nullptr)
		m_position = node->GetCenter();
}

Driver::~Driver()
{
}

void Driver::Next()
{
	if (m_nodeCurrent == nullptr)
		return;

	m_connection = nullptr;
	m_nodeTarget = nullptr;

	NodeGroup* nodeGroup = m_nodeCurrent->GetNodeGroup();
	Array<NodeGroupConnection*> possibleConnections;
	for (NodeGroupConnection* connection : nodeGroup->GetOutputs())
	{
		const NodeSubGroup& group = connection->GetInput();
		if (m_nodeCurrent->GetIndex() >= group.index &&
			m_nodeCurrent->GetIndex() < group.index + group.count)
		{
			possibleConnections.push_back(connection);
		}
	}
	if (possibleConnections.size() == 0)
		return;

	int index = Random::NextInt((int) possibleConnections.size());
	m_connection = possibleConnections[index];
	if (m_connection->GetDrivingLines().size() == 0)
	{
		m_connection = nullptr;
		return;
	}

	int indexInConnection = m_nodeCurrent->GetIndex() - m_connection->GetInput().index;
	m_drivingLineCurrent = m_connection->GetDrivingLines()[indexInConnection];

	NodeSubGroup output = m_connection->GetOutput();
	index = indexInConnection - 1 + Random::NextInt(3);
	index = Math::Clamp(index, 0, output.count - 1);
	m_nodeTarget = output.GetNode(index);
	m_drivingLineTarget = m_connection->GetDrivingLines()[index];
}

void Driver::Update(float dt)
{
	if (m_nodeTarget == nullptr)
	{
		Next();
	}
	else
	{
		m_distance += m_speed * dt;

		Meters length = m_drivingLineTarget.Length();

		m_direction = Vector2f(1.0f, 0.0f);

		if (m_distance >= length)
		{
			m_distance -= length;
			m_position.xy = m_drivingLineTarget.second.end;
			m_position.z = m_nodeTarget->GetPosition().z;
			m_nodeCurrent = m_nodeTarget;
			m_nodeTarget = nullptr;
			m_connection = nullptr;
			Next();
		}

		if (m_nodeTarget != nullptr)
		{
			length = m_drivingLineTarget.Length();
			float t = m_distance / length;
			m_position.xy = m_drivingLineCurrent.GetPoint(m_distance);
			m_position.xy = Vector2f::Lerp(m_position.xy,
				m_drivingLineTarget.GetPoint(m_distance), t);
			m_position.z = Math::Lerp(
				m_nodeCurrent->GetPosition().z,
				m_nodeTarget->GetPosition().z,
				t);
		}
	}
}

