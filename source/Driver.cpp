#include "Driver.h"
#include "DrivingSystem.h"


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

Driver::Driver(RoadNetwork* network, DrivingSystem* drivingSystem, Node* node, int id)
	: m_nodeCurrent(node)
	, m_roadNetwork(network)
	, m_drivingSystem(drivingSystem)
	, m_distance(0.0f)
	, m_speed(0.0f)
	, m_acceleration(0.0f)
	, m_desiredSpeed(20.0f)
	, m_destroy(false)
	, m_speedPrev(0.0f)
	, m_brakeLightTimer(0.0f)
	, m_blinkerTimer(0.0f)
	, m_id(id)
{
	m_desiredSpeed = Random::NextFloat(10.0f, 20.0f);
	m_speed = m_desiredSpeed;
	m_distance = Random::NextFloat(0.0f, 3.0f);

	DriverVehicleParams params;
	Array<DriverVehicleParams> vehicles;

	// Normal Car
	params.trailerCount = 1;
	params.acceleration = 20.0f;
	params.deceleration = 70.0f;
	params.size[0] = Vector3f(4.78f, 1.96f, 1.18f);
	params.maxSpeed = 25.0f;
	vehicles.push_back(params);
	vehicles.push_back(params);
	vehicles.push_back(params);
	vehicles.push_back(params);
	vehicles.push_back(params);
	vehicles.push_back(params);
	vehicles.push_back(params);
	vehicles.push_back(params);

	// Bus
	params.trailerCount = 1;
	params.size[0] = Vector3f(9.0f, 2.1f, 3.5f);
	vehicles.push_back(params);
	vehicles.push_back(params);

	// 1-Trailer Truck
	params.trailerCount = 2;
	params.size[0] = Vector3f(8.1f, 2.4f, 3.8f);
	params.size[1] = Vector3f(12.7f, 2.8f, 4.0f);
	params.pivotOffset[0] = -1.7f;
	vehicles.push_back(params);
	vehicles.push_back(params);
	
	// 2-Trailer Truck
	params.trailerCount = 3;
	params.size[0] = Vector3f(3.0f, 2.4f, 3.8f);
	params.size[1] = Vector3f(7.5f, 2.5f, 4.0f);
	params.size[2] = Vector3f(7.5f, 2.5f, 4.0f);
	params.pivotOffset[0] = 0.4f;
	params.pivotOffset[1] = 0.4f;
	vehicles.push_back(params);

	m_vehicleParams = Random::Choose(vehicles);


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
	if (m_surface != nullptr) 
		m_surface->RemoveDriver(this);
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
	if (next.GetSurface() != nullptr)
		m_path.push_back(next);
}

DriverPathNode Driver::Next(Node* node)
{
	NodeGroup* nodeGroup = node->GetNodeGroup();

	if (nodeGroup->GetIntersection() != nullptr &&
		nodeGroup->GetOutputs().size() == 0)
	{
		RoadIntersection* intersection = nodeGroup->GetIntersection();
		Array<NodeGroup*> possibleGroups;
		for (RoadIntersectionPoint* point : intersection->GetPoints())
		{
			if (point->GetIOType() == IOType::OUTPUT)
				possibleGroups.push_back(point->GetNodeGroup());
		}
		if (possibleGroups.size() > 0)
		{
			NodeGroup* nextGroup = Random::Choose(possibleGroups);
			Node* nextNode = nextGroup->GetNode(Random::NextInt(nextGroup->GetNumNodes()));
			return DriverPathNode(intersection, node, nextNode);
		}
		else
		{
			return DriverPathNode();
		}
	}

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
	if (m_surface != nullptr)
		CheckAvoidance(m_surface);
	for (NodeGroupConnection* connection : node->GetNodeGroup()->GetOutputs())
	{
		if (connection != connection && connection->GetInput().ContainsNode(node))
			CheckAvoidance(connection);
	}
	node = current.GetEndNode();
	for (NodeGroupConnection* connection : node->GetNodeGroup()->GetInputs())
	{
		if (connection->GetOutput().ContainsNode(node) &&
			connection->GetInput().group != current.GetStartNode()->GetNodeGroup())
			CheckAvoidance(connection);
	}
	if (node->GetNodeGroup()->GetIntersection() != nullptr)
		CheckAvoidance(node->GetNodeGroup()->GetIntersection());
	node = current.GetEndNode();
	for (NodeGroupConnection* connection : node->GetNodeGroup()->GetOutputs())
	{
		if (connection->GetInput().ContainsNode(node))
			CheckAvoidance(connection);
	}
}

void Driver::CheckAvoidance(RoadSurface* surface)
{
	for (Driver* driver : surface->GetDrivers())
	{
		if (driver != this)
			CheckAvoidance(driver);
	}
}

void Driver::CheckAvoidance(Driver* driver)
{
	Meters timeOfImpact = -1.0f;
	
	// Make sure we are behind the other driver
	Vector3f front0 = GetFrontPostion();
	Vector3f front1 = driver->GetFrontPostion();
	float bd = front1.xy.Dot(GetDirection()) - front0.xy.Dot(GetDirection());
	float ad = front0.xy.Dot(driver->GetDirection()) - front1.xy.Dot(driver->GetDirection());
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
			m_surface->RemoveDriver(this);
			m_surface = nullptr;
			Next();
			if (m_path.size() == 0)
				m_destroy = true;
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
			if (m_surface != pathNode.GetSurface())
			{
				if (m_surface != nullptr) 
					m_surface->RemoveDriver(this);
				m_surface = pathNode.GetSurface();
					m_surface->AddDriver(this);
			}
		}
	}
	else
	{
		Next();
	}

	if (m_path.size() == 0 && m_surface != nullptr) 
		m_surface->RemoveDriver(this);

	// Update brake light state

	m_speedSamples.push_back(m_speed);
	if (m_speedSamples.size() > 8)
		m_speedSamples.erase(m_speedSamples.begin());
	MetersPerSecond avgSpeed = 0.0f;
	for (float sample : m_speedSamples)
		avgSpeed += sample;
	avgSpeed /= (MetersPerSecond) m_speedSamples.size();
	float deltaSpeed = (avgSpeed - m_speedPrev) / dt;
	m_speedPrev = avgSpeed;
	
	m_lightState.leftBlinker = false;
	m_lightState.rightBlinker = false;
	if (m_path.size() > 0)
	{
		int laneShift = m_path[0].GetLaneShift();
		if (laneShift < 0)
			m_lightState.leftBlinker = m_blinkerTimer >= 0.0f;
		else if (laneShift > 0)
			m_lightState.rightBlinker = m_blinkerTimer >= 0.0f;
	}
	m_blinkerTimer -= dt;
	if (m_blinkerTimer < -0.3f)
		m_blinkerTimer += 0.6f;

	//if ((m_speed < mphToMetersPerSecond(5.0f) && deltaSpeed < FLT_EPSILON) ||
	//	deltaSpeed < -1.0f)
	if (deltaSpeed < -20.0f)
		m_brakeLightTimer = 0.5f;
	if (m_brakeLightTimer > 0.0f)
	{
		m_brakeLightTimer -= dt;
		m_lightState.braking = true;
	}
	else
	{
		m_lightState.braking = false;
	}
	m_speedPrev = m_speed;

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
