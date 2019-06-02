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
	, m_roadNetwork(network)
	, m_distance(0.0f)
	, m_speed(0.0f)
	, m_acceleration(0.0f)
	, m_desiredSpeed(20.0f)
{
	m_desiredSpeed = Random::NextFloat(10.0f, 20.0f);
	m_distance = Random::NextFloat(0.0f, 3.0f);

	m_vehicleParams.trailerCount = 1;
	m_vehicleParams.acceleration = 20.0f;
	m_vehicleParams.deceleration = 70.0f;
	m_vehicleParams.size[0] = Vector3f(4.78f, 1.96f, 1.18f);
	m_vehicleParams.maxSpeed = 25.0f;

	if (Random::NextInt(10) > 7)
	{
		m_vehicleParams.trailerCount = 2;
		m_vehicleParams.size[0] = Vector3f(8.1f, 2.4f, 3.8f);
		m_vehicleParams.size[1] = Vector3f(12.7f, 2.8f, 4.0f);
		m_vehicleParams.pivotOffset[0] = -1.7f;
		m_vehicleParams.pivotOffset[1] = -1.7f;
	}

	if (node != nullptr)
	{
		m_position = node->GetCenter();
		m_futureStates[0].position[0] = m_position;
		m_futureStates[0].direction[0] = Vector2f::UNITX;
		for (int i = 1; i < m_vehicleParams.trailerCount; i++)
		{
			m_futureStates[0].position[i] = m_futureStates[0].position[i - 1] -
				Vector3f((m_vehicleParams.size[i - 1].x +
					m_vehicleParams.size[i].x) * 0.5f -
					m_vehicleParams.pivotOffset[i - 1], 0.0f, 0.0f);
			m_futureStates[0].direction[i] = Vector2f::UNITX;
		}
	}
}

Driver::~Driver()
{
	if (m_connection != nullptr) 
		m_connection->RemoveDriver(this);
}

bool Driver::GetFuturePosition(Meters distance, Vector3f& position, Vector2f& direction)
{
	Meters currentDistance = -m_distance;
	for (unsigned int i = 0; i < m_path.size(); i++)
	{
		DriverPathNode pathNode = m_path[i];

		if (currentDistance + pathNode.GetDistance() >= distance)
		{
			Meters distOnNode = distance - currentDistance;
			position = Vector3f::ZERO;
			position.xy = pathNode.GetDrivingLine().GetPoint(distOnNode);
			direction = pathNode.GetDrivingLine().GetTangent(distOnNode);
			return true;
		}

		currentDistance += pathNode.GetDistance();
	}
	return false;
}

void Driver::Next()
{
	Node* node = m_nodeCurrent;
	if (m_path.size() > 0)
		node = m_path.back().GetEndNode();
	DriverPathNode next = Next(node);
	if (next.GetConnection() != nullptr)
		m_path.push_back(next);
}

DriverPathNode Driver::Next(Node* node)
{
	NodeGroup* nodeGroup = node->GetNodeGroup();
	Array<NodeGroupConnection*> possibleConnections;
	for (NodeGroupConnection* connection : nodeGroup->GetOutputs())
	{
		const NodeSubGroup& group = connection->GetInput();
		if (node->GetIndex() >= group.index &&
			node->GetIndex() < group.index + group.count)
		{
			possibleConnections.push_back(connection);
		}
	}
	if (possibleConnections.size() == 0)
		return DriverPathNode();

	int index = Random::NextInt((int) possibleConnections.size());
	NodeGroupConnection* connection = possibleConnections[index];
	if (connection->GetDrivingLines().size() == 0)
	{
		connection = nullptr;
		return DriverPathNode();
	}

	int fromLaneIndex = node->GetIndex() -
		connection->GetInput().index;
	NodeSubGroup output = connection->GetOutput();
	int toLaneindex = fromLaneIndex - 1 + Random::NextInt(3);
	toLaneindex = Math::Clamp(toLaneindex, 0, output.count - 1);
	return DriverPathNode(connection, fromLaneIndex, toLaneindex);
}

void Driver::CheckAvoidance()
{
	if (m_path.size() == 0)
		return;

	DriverPathNode current = m_path[0];

	Node* node = current.GetStartNode();
	for (NodeGroupConnection* connection : node->GetNodeGroup()->GetOutputs())
	{
		if (connection->GetInput().ContainsNode(node))
			CheckAvoidance(connection);
	}
	node = current.GetEndNode();
	for (NodeGroupConnection* connection : node->GetNodeGroup()->GetInputs())
	{
		if (connection->GetOutput().ContainsNode(node) &&
			connection->GetInput().group != current.GetStartNode()->GetNodeGroup())
			CheckAvoidance(connection);
	}
	node = current.GetEndNode();
	for (NodeGroupConnection* connection : node->GetNodeGroup()->GetOutputs())
	{
		if (connection->GetInput().ContainsNode(node))
			CheckAvoidance(connection);
	}
}

void Driver::CheckAvoidance(NodeGroupConnection* connection)
{
	for (Driver* driver : connection->GetDrivers())
	{
		if (driver != this)
		{
			CheckAvoidance(driver);
		}
	}
}

void Driver::CheckAvoidance(Driver* driver)
{
	Meters timeOfImpact = -1.0f;
	
	// Make sure we are behind the other driver
	float bd = driver->GetPosition().xy.Dot(GetDirection()) -
		GetPosition().xy.Dot(GetDirection());
	float ad = GetPosition().xy.Dot(driver->GetDirection()) -
		driver->GetPosition().xy.Dot(driver->GetDirection());
	if (bd < ad)
		return;

	// Determine time of collision
	for (unsigned int i = 0; i < DRIVER_MAX_FUTURE_STATES; i++)
	{
		if (Driver::CheckCollision(
				m_vehicleParams, m_futureStates[i],
				driver->m_vehicleParams, driver->m_futureStates[0]) ||
			(i > 0 && Driver::CheckCollision(
				m_vehicleParams, m_futureStates[i],
				driver->m_vehicleParams, driver->m_futureStates[i])))
		{
			timeOfImpact = driver->m_futureStates[i].time;
			if (i == 0)
				m_isColliding = true;
			break;
		}
	}

	Meters distOfImpact = timeOfImpact * m_speed;
	if (timeOfImpact < 0.0f)
		return;
	if (timeOfImpact > DRIVER_COLLISION_LOOK_AHEAD)
		return;

	m_collisions.push_back(driver);
	m_acceleration = -m_vehicleParams.deceleration;
	if (m_isColliding)
		m_speed = 0.0f;
}

bool Driver::CheckCollision(
	const DriverVehicleParams& paramsA,
	const DriverCollisionState& a,
	const DriverVehicleParams& paramsB,
	const DriverCollisionState& b)
{
	Vector2f c1, c2, c3, c4, mins, maxs;
	for (int i = 0; i < paramsA.trailerCount; i++)
	{
		Vector2f posA = a.position[i].xy;
		Vector2f dirA = a.direction[i];
		Vector2f sizeA = paramsA.size[i].xy * 0.5f;

		for (int j = 0; j < paramsB.trailerCount; j++)
		{
			Vector2f posB = b.position[j].xy;
			Vector2f dirB = b.direction[j];
			Vector2f sizeB = paramsB.size[j].xy * 0.5f;
			
			if (posA.DistToSqr(posB) > sizeA.LengthSquared() + sizeB.LengthSquared())
				continue;

			bool touching = true;

			Matrix3f t = b.GetTransformInv(j) * a.GetTransform(i);
			c1 = t.TransformVector(Vector2f(sizeA.x, sizeA.y));
			c2 = t.TransformVector(Vector2f(sizeA.x, -sizeA.y));
			c3 = t.TransformVector(Vector2f(-sizeA.x, sizeA.y));
			c4 = t.TransformVector(Vector2f(-sizeA.x, -sizeA.y));
			mins.x = Math::Min(Math::Min(c1.x, c2.x), Math::Min(c3.x, c4.x));
			mins.y = Math::Min(Math::Min(c1.y, c2.y), Math::Min(c3.y, c4.y));
			maxs.x = Math::Max(Math::Max(c1.x, c2.x), Math::Max(c3.x, c4.x));
			maxs.y = Math::Max(Math::Max(c1.y, c2.y), Math::Max(c3.y, c4.y));
			if (maxs.x < -sizeB.x || mins.x > sizeB.x ||
				maxs.y < -sizeB.y || mins.y > sizeB.y)
				touching =false;

			t = a.GetTransformInv(i) * b.GetTransform(j);
			c1 = t.TransformVector(Vector2f(sizeB.x, sizeB.y));
			c2 = t.TransformVector(Vector2f(sizeB.x, -sizeB.y));
			c3 = t.TransformVector(Vector2f(-sizeB.x, sizeB.y));
			c4 = t.TransformVector(Vector2f(-sizeB.x, -sizeB.y));
			mins.x = Math::Min(Math::Min(c1.x, c2.x), Math::Min(c3.x, c4.x));
			mins.y = Math::Min(Math::Min(c1.y, c2.y), Math::Min(c3.y, c4.y));
			maxs.x = Math::Max(Math::Max(c1.x, c2.x), Math::Max(c3.x, c4.x));
			maxs.y = Math::Max(Math::Max(c1.y, c2.y), Math::Max(c3.y, c4.y));
			if (maxs.x < -sizeA.x || mins.x > sizeA.x ||
				maxs.y < -sizeA.y || mins.y > sizeA.y)
				touching = false;

			if (touching)
				return true;
		}
	}
	return false;

	Meters radius = 2.5f;
	Vector2f pos0 = a.position[0].xy;
	Vector2f pos1 = b.position[0].xy;
	return (Vector2f::Dist(pos0, pos1) < radius);
}

void Driver::Update(float dt)
{
	m_speed += m_acceleration * dt;
	if (m_speed < 0.0f)
		m_speed = 0.0f;

	if (m_path.size() > 0)
	{
		if (m_path.size() < 4)
			Next();

		m_distance += m_speed * dt;

		DriverPathNode pathNode = m_path.front();
		BiarcPair drivingLine = pathNode.GetDrivingLine();
		Meters length = drivingLine.Length();

		if (m_distance >= length)
		{
			m_distance -= length;
			m_position.xy = drivingLine.second.end;
			m_position.z = pathNode.GetEndNode()->GetPosition().z;
			m_nodeCurrent = pathNode.GetEndNode();
			m_path.erase(m_path.begin());
			m_connection->RemoveDriver(this);
			m_connection = nullptr;
			Next();
		}

		if (m_path.size() > 0)
		{
			pathNode = m_path.front();
			drivingLine = pathNode.GetDrivingLine();
			length = drivingLine.Length();
			float t = m_distance / length;
			m_position.xy = drivingLine.GetPoint(m_distance);
			m_direction = drivingLine.GetTangent(m_distance);
			m_position.z = Math::Lerp(
				pathNode.GetStartNode()->GetPosition().z,
				pathNode.GetEndNode()->GetPosition().z,
				t);
			if (m_connection != pathNode.GetConnection())
			{
				if (m_connection != nullptr) 
					m_connection->RemoveDriver(this);
				m_connection = pathNode.GetConnection();
					m_connection->AddDriver(this);
			}
		}
	}
	else
	{
		Next();
	}

	if (m_path.size() == 0 && m_connection != nullptr) 
		m_connection->RemoveDriver(this);

	UpdateFutureStates();
}

void Driver::UpdateFutureStates()
{
	DriverCollisionState prevState = m_futureStates[0];

	m_futureStates[0].time = 0.0f;
	m_futureStates[0].position[0] = m_position;
	m_futureStates[0].direction[0] = m_direction;
	m_futureStates[0].count = m_vehicleParams.trailerCount;

	Meters currentDistance = -m_distance;
	unsigned int pathIndex = 0;

	for (int futureIndex = 1; futureIndex < DRIVER_MAX_FUTURE_STATES; futureIndex++)
	{
		DriverCollisionState& state = m_futureStates[futureIndex];
		state = m_futureStates[futureIndex - 1];
		state.time += DRIVER_FUTURE_STATE_TIME_DELTA;
		float distance = state.time * m_speed;

		for (; pathIndex < m_path.size(); pathIndex++)
		{
			if (currentDistance + m_path[pathIndex].GetDistance() >= distance)
			{
				Meters distOnNode = distance - currentDistance;
				state.position[0] = Vector3f::ZERO;
				state.position[0].xy =
					m_path[pathIndex].GetDrivingLine().GetPoint(distOnNode);
				state.direction[0] = 
					m_path[pathIndex].GetDrivingLine().GetTangent(distOnNode);
				break;
			}
			currentDistance += m_path[pathIndex].GetDistance();
		}

		// Update trailer positions/orientations
		for (int futureIndex = 0; futureIndex < DRIVER_MAX_FUTURE_STATES; futureIndex++)
		{
			DriverCollisionState& state = m_futureStates[futureIndex];
			DriverCollisionState* prevTrailerState = &prevState;
			if (futureIndex > 0)
				prevTrailerState = &m_futureStates[futureIndex - 1];
			state.count = m_vehicleParams.trailerCount;

			for (int i = 1; i < m_vehicleParams.trailerCount; i++)
			{
				Vector2f a = state.position[i - 1].xy -
					state.direction[i - 1] * ((m_vehicleParams.size[i - 1].x * 0.5f) +
						m_vehicleParams.pivotOffset[i - 1]);
				Vector2f b = (prevTrailerState->position[i].xy -
					prevTrailerState->direction[i] * ((m_vehicleParams.size[i].x * 0.25f) +
						m_vehicleParams.pivotOffset[i - 1]));
				state.direction[i] = Vector2f::Normalize(a - b);
				state.position[i].xy = a - state.direction[i] *
					((m_vehicleParams.size[i].x * 0.5f) +
						m_vehicleParams.pivotOffset[i - 1]);
				state.position[i].z = state.position[0].z;
			}
		}
	}
}

void Driver::IntegrateVelocity(float dt)
{
	if (m_path.size() == 0)
		return;

	if (m_speed < m_desiredSpeed)
		m_acceleration = m_vehicleParams.acceleration;
	else if (m_speed > m_desiredSpeed)
		m_acceleration = -m_vehicleParams.deceleration;
	if (Math::Abs(m_speed - m_desiredSpeed) < m_acceleration * dt)
		m_acceleration = 0.0f;

	// Reset collision debug info
	m_isColliding = false;
	m_collisions.clear();

	UpdateFutureStates();
}
