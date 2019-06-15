#pragma once

#include "NodeGroupConnection.h"
#include "DriverPath.h"

class DrivingSystem;


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
			Vector3f(direction[index].y, -direction[index].x, 0.0f),
			Vector3f::UNITZ);
		return Matrix3f::CreateTranslation(position[index].xy) * dcm;
	}

	inline Matrix3f GetTransformInv(int index = 0) const
	{
		Matrix3f dcm = Matrix3f(
			Vector3f(direction[index].x, direction[index].y, 0.0f),
			Vector3f(direction[index].y, -direction[index].x, 0.0f),
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

struct DriverLightState
{
	bool braking;
	bool leftBlinker;
	bool rightBlinker;
};

enum class DriverState
{
	DRIVING = 0,
	STOPPING = 1,
	STOPPED = 2,
};

class Driver
{
public:
	friend class DrivingSystem;

public:
	Driver();
	Driver(RoadNetwork* network, DrivingSystem* drivingSystem, Node* node, int id);
	~Driver();

	inline Vector3f GetFrontPostion() const
	{
		return Vector3f(m_position.xy + (m_direction *
			m_vehicleParams.size[0].x * 0.5f), m_position.z);
	}

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

	inline const RoadCurveLine& GetDrivingLine() const
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
	inline DriverState GetMovementState() const
	{
		return m_state;
	}

	inline const Matrix3f& GetOrientation() const
	{
		return m_orientation;
	}

	inline void Push(Meters amount) { m_distance = Math::Max(0.0f, m_distance + amount); }
	inline bool IsColliding() const { return m_isColliding; }
	inline const DriverLightState& GetLightState() const { return m_lightState; }
	inline int GetId() const { return m_id; }

	bool GetFuturePosition(Meters distance, Vector3f& position, Vector2f& direction);
	void GetNextStop(Meters& outDistance, Node*& outNode, TrafficLightSignal& outSignal);

	void Next();
	DriverPathNode Next(Node* node);
	void CheckAvoidance();
	void CheckAvoidance(RoadSurface* surface);
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
	int m_id;
	Node* m_nodeCurrent;
	int m_laneIndexCurrent;
	int m_laneIndexTarget; // Relative to node group lanes
	RoadNetwork* m_roadNetwork;
	DrivingSystem* m_drivingSystem;
	Array<DriverPathNode> m_path;
	RoadSurface* m_surface;
	bool m_destroy;
	DriverLightState m_lightState;
	Seconds m_brakeLightTimer;
	Seconds m_blinkerTimer;
	MetersPerSecond m_speedPrev;
	Array<MetersPerSecond> m_speedSamples;

	DriverState m_state;
	Seconds m_stopTimer;
	Node* m_currentStopNode;

	Meters m_distance;
	MetersPerSecond m_speed;
	MetersPerSecondSq m_acceleration;
	Vector3f m_position;
	Vector2f m_direction;
	Vector2f m_velocity;
	Matrix3f m_orientation;
	DriverCollisionState m_futureStates[DRIVER_MAX_FUTURE_STATES];

	DriverVehicleParams m_vehicleParams; // length, width, height

	MetersPerSecond m_desiredSpeed;
public:

	// Debug
	Array<Driver*> m_collisions;
	int m_collisionIndex;
	bool m_futureCollision;
	bool m_isColliding;
};

