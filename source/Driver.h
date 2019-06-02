#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "NodeGroupConnection.h"

class DriverPathNode
{
public:
	DriverPathNode()
		: m_connection(nullptr)
	{}
	DriverPathNode(NodeGroupConnection* connection,
		int startLaneIndex, int endLaneIndex) 
		: m_connection(connection)
		, m_laneIndexStart(startLaneIndex)
		, m_laneIndexEnd(endLaneIndex)
	{
		m_drivingLine = m_connection->GetDrivingLine(
			startLaneIndex, endLaneIndex);
	}

	inline Node* GetStartNode() const {
		return m_connection->GetInput().GetNode(m_laneIndexStart);
	}
	inline Node* GetEndNode() const {
		return m_connection->GetOutput().GetNode(m_laneIndexEnd);
	}
	inline NodeGroupConnection* GetConnection() const {
		return m_connection;
	}
	inline const BiarcPair& GetDrivingLine() const {
		return m_drivingLine;
	}
	inline Meters GetDistance() const {
		return m_drivingLine.Length();
	}

private:
	NodeGroupConnection* m_connection;
	int m_laneIndexStart;
	int m_laneIndexEnd; // Relative to connection left lane
	BiarcPair m_drivingLine;
};

constexpr auto MAX_VEHICLE_TRAILERS = 3;
constexpr auto DRIVER_MAX_FUTURE_STATES = 8;
constexpr Seconds DRIVER_FUTURE_STATE_TIME_DELTA = 0.25f;
constexpr Seconds DRIVER_COLLISION_LOOK_AHEAD = 1.0f;


struct DriverCollisionState
{
	Vector3f position[MAX_VEHICLE_TRAILERS];
	Vector2f direction[MAX_VEHICLE_TRAILERS];
	int count;
	Seconds time;

	inline Matrix3f GetTransform(int index = 0) const
	{
		Matrix3f dcm = Matrix3f(
			Vector3f(direction[index].x, direction[index].y, 0.0f),
			Vector3f(-direction[index].y, direction[index].x, 0.0f),
			Vector3f::UNITZ);
		return Matrix3f::CreateTranslation(position[index].xy) * dcm;
	}

	inline Matrix3f GetTransformInv(int index = 0) const
	{
		Matrix3f dcm = Matrix3f(
			Vector3f(direction[index].x, direction[index].y, 0.0f),
			Vector3f(-direction[index].y, direction[index].x, 0.0f),
			Vector3f::UNITZ);
		return dcm.GetTranspose() * Matrix3f::CreateTranslation(-position[index].xy);
	}
};

struct DriverVehicleParams
{
	int trailerCount;
	Vector3f size[MAX_VEHICLE_TRAILERS];
	Meters pivotOffset[MAX_VEHICLE_TRAILERS];
	MetersPerSecondSq acceleration;
	MetersPerSecondSq deceleration;
	MetersPerSecondSq maxSpeed;
};

class Driver
{
public:
	Driver();
	Driver(RoadNetwork* network, Node* node);
	~Driver();

	inline const Vector3f& GetPosition() const
	{
		return m_position;
	}

	inline const Vector2f& GetDirection() const
	{
		return m_direction;
	}

	inline Vector2f GetVelocity() const
	{
		return m_direction * m_speed;
	}

	inline float GetSlowDownPercent() const
	{
		return 1.0f - (m_speed / m_desiredSpeed);
	}

	inline MetersPerSecond GetSpeed() const
	{
		return m_speed;
	}

	inline MetersPerSecondSq GetAcceleration() const
	{
		return m_acceleration;
	}

	inline void SetSpeed(MetersPerSecond speed)
	{
		m_speed = speed;
	}

	inline const BiarcPair& GetDrivingLine() const
	{
		return m_path[0].GetDrivingLine();
	}

	inline const Array<DriverPathNode>& GetPath() const
	{
		return m_path;
	}

	inline const DriverVehicleParams& GetVehicleParams() const
	{
		return m_vehicleParams;
	}

	inline const DriverCollisionState& GetState(int index = 0) const
	{
		return m_futureStates[index];
	}

	inline void Push(Meters amount)
	{
		m_distance = Math::Max(0.0f, m_distance + amount);
	}

	inline bool IsColliding() const
	{
		return m_isColliding;
	}

	bool GetFuturePosition(Meters distance, Vector3f& position, Vector2f& direction);

	void Next();
	DriverPathNode Next(Node* node);
	void CheckAvoidance();
	void CheckAvoidance(NodeGroupConnection* connection);
	void CheckAvoidance(Driver* driver);
	void Update(float dt);
	void UpdateFutureStates();
	void IntegrateVelocity(float dt);

	static bool CheckCollision(
		const DriverVehicleParams& paramsA,
		const DriverCollisionState& a,
		const DriverVehicleParams& paramsB,
		const DriverCollisionState& b);

private:
	Node* m_nodeCurrent;
	int m_laneIndexCurrent;
	int m_laneIndexTarget; // Relative to node group lanes
	RoadNetwork* m_roadNetwork;
	Array<DriverPathNode> m_path;
	NodeGroupConnection* m_connection;

	Meters m_distance;
	MetersPerSecond m_speed;
	MetersPerSecondSq m_acceleration;
	Vector3f m_position;
	Vector2f m_direction;
	Vector2f m_velocity;
	DriverCollisionState m_futureStates[DRIVER_MAX_FUTURE_STATES];

	DriverVehicleParams m_vehicleParams;

	MetersPerSecond m_desiredSpeed;
public:
	Array<Driver*> m_collisions;
	bool m_isColliding;
};


#endif // _DRIVER_H_