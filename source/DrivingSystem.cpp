#include "DrivingSystem.h"
#include "RoadNetwork.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

DrivingSystem::DrivingSystem(RoadNetwork* network)
	: m_network(network)
{
	m_trafficPercent = 0.0f;
	m_driverIdCounter = 1;
}

DrivingSystem::~DrivingSystem()
{
	Clear();
}

void DrivingSystem::Clear()
{
	for (Driver* driver : m_drivers)
		delete driver;
	m_drivers.clear();
}

float DrivingSystem::GetTrafficPercent()
{
	return m_trafficPercent;
}

void DrivingSystem::SpawnDriver()
{
	/*NodeGroup* nodeGroup = Random::ChooseFromSet(m_network->GetNodeGroups());
	int index = Random::NextInt(nodeGroup->GetNumNodes());
	Node* node = nodeGroup->GetNode(index);
	Driver* driver = new Driver(m_network, node);
	m_drivers.push_back(driver);*/

	Set<Node*> nodes;
	for (NodeGroupConnection* connection : m_network->GetNodeGroupConnections())
	{
		if (!connection->IsGhost())
		{
			for (int i = 0; i < connection->GetInput().count; i++)
			{
				NodeGroup* nodeGroup = connection->GetInput().group;
				if (nodeGroup->GetInputs().size() == 0 &&
						nodeGroup->GetIntersection(IOType::INPUT) == nullptr)
					nodes.insert(connection->GetInput().GetNode(i));
			}
		}
	}
	if (nodes.size() > 0)
	{
		Node* node = Random::ChooseFromSet(nodes);
		Driver* driver = new Driver(m_network, this, node, m_driverIdCounter);
		m_driverIdCounter++;
		m_drivers.push_back(driver);
	}
}

void DrivingSystem::DeleteDriver(Driver* driver)
{
	auto it = std::find(m_drivers.begin(), m_drivers.end(), driver);
	if (it != m_drivers.end())
		m_drivers.erase(it);
	delete driver;
}


void DrivingSystem::Update(float dt)
{
	int destroyCount = 0;
	for (unsigned int i = 0; i < m_drivers.size(); i++)
	{
		if (m_drivers[i]->m_destroy)
		{
			delete m_drivers[i];
			m_drivers.erase(m_drivers.begin() + i);
			i--;
			destroyCount++;
		}
	}
	for (int i = 0; i < destroyCount; i++)
		SpawnDriver();

	for (Driver* driver : m_drivers)
		driver->IntegrateVelocity(dt);
	for (Driver* driver : m_drivers)
		driver->CheckAvoidance();
	for (Driver* driver : m_drivers)
		driver->Update(dt);
	m_trafficPercent = 0.0f;
	for (Driver* driver : m_drivers)
		m_trafficPercent += driver->GetSlowDownPercent();
	if (m_drivers.size() > 0)
		m_trafficPercent /= m_drivers.size();

	for (Driver* a: m_drivers)
	{
		for (Driver* b: m_drivers)
		{
			Driver* fast;
			Driver* slow;
			float bd = b->GetPosition().xy.Dot(a->GetDirection()) - a->GetPosition().xy.Dot(a->GetDirection());
			float ad = a->GetPosition().xy.Dot(b->GetDirection()) - b->GetPosition().xy.Dot(b->GetDirection());
			if (bd > ad)
			{
				fast = a;
				slow = b;
			}
			else
			{
				fast = b;
				slow = a;
			}
			Meters dist = Vector2f::Dist(a->GetPosition().xy, b->GetPosition().xy);
			if (dist < 1.0f)
			{
				fast->Push(-1.0f * dt);
				slow->Push(1.0f * dt);
			}
			continue;

			if (dist < 3.0)
			{
				Vector2f v = slow->GetPosition().xy - fast->GetPosition().xy;
				v.Normalize();
				Vector2f dx = slow->GetPosition().xy - fast->GetPosition().xy;
				Vector2f dv = fast->GetVelocity() - slow->GetVelocity();
				float dot = Vector2f::Dot(dx, dv);
				if (dot > 0.0f)
				{
					float slowDown = dv.Dot(v) / Vector2f::Normalize(fast->GetVelocity()).Dot(v);
					if (dist > 3.0)
						slowDown = fast->GetVehicleParams().deceleration * dt;
					else
						slowDown += 1.0f;
					fast->SetSpeed(Math::Max(0.0f, fast->GetSpeed() - slowDown));
					if (dist < 1.5f)
					{
						fast->Push(-1.0f * dt);
						slow->Push(1.0f * dt);
					}
				}
			}
		}
	}
}
