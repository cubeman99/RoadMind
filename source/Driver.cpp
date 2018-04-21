#include "Driver.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Driver::Driver()
	: m_lane(nullptr)
	, m_velocity(Vector2f::ZERO)
	, m_direction(Vector2f::UNITX)
	, m_roadNetwork(nullptr)
	, m_speed(10.0f)
{
	m_speed = 10.0f;
}

Driver::Driver(RoadNetwork* network, Node* node)
	: m_lane(nullptr)
	, m_roadNetwork(network)
	, m_speed(10.0f)
{
	if (node != nullptr && node->GetNumOutputs() > 0)
		m_lane = *node->GetOutputs().begin();
	m_distance = 0.0f;
	if (node != nullptr)
		m_position = node->GetCenter();
}

Driver::~Driver()
{
}

void Driver::Update(float dt)
{
	if (m_lane != nullptr)
	{
		m_distance += m_speed * dt;

		float laneDistance = m_lane->GetDistance();
		if (m_distance >= laneDistance)
		{
			Node* node = m_lane->GetOutput();
			if (node->GetNumOutputs() > 0)
			{
				int index = Random::NextInt(node->GetNumOutputs());
				auto it = node->GetOutputs().begin();
				for (int i = 0; i < index; i++)
					it++;
				m_lane = *it;
				m_distance -= laneDistance;
			}
			else
			{
				m_distance = laneDistance;
			}
		}

		m_position = m_lane->GetPoint(m_distance, LaneSide::CENTER, 0.0f);
	}
}

