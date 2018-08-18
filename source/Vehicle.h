#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#include <cmgGraphics/cmg_graphics.h>
#include <cmgPhysics/cmg_physics.h>
#include "CommonTypes.h"
#include "VehicleParams.h"

class Vehicle;


class Wheel
{
public:
	Wheel()
	{
		m_speed = 0.0f;
		m_angle = 0.0f;
	}

	inline bool IsOnGround() const
	{
		return true;
	}

	//inline const WheelSuspensionState& GetSuspensionState() const { return m_suspensionState; }
	inline Vector3f GetGroundPosition() const
	{
		return m_groundPosition;
	}

	inline const VehicleAxleParams& GetAxleParams() const
	{
		return *m_axleParams;
	}
	inline unsigned int GetAxleIndex() const
	{
		return m_axleIndex;
	}
	inline unsigned int GetSideOfAxle() const
	{
		return m_sideOfAxle;
	}
	inline Radians GetAngle() const
	{
		return m_angle;
	}
	inline const Matrix4f& GetWheelToBody() const
	{
		return m_wheelToBody;
	}
	inline const Matrix4f& GetBodyToWheel() const
	{
		return m_bodyToWheel;
	}
	inline const Matrix4f& GetWheelToWorld() const
	{
		return m_wheelToWorld;
	}
	inline const Matrix4f& GetWorldToWheel() const
	{
		return m_worldToWheel;
	}
	inline float GetRadius() const
	{
		return m_axleParams->wheels.radius;
	}
	//inline float GetAngle() const { return m_angle; } 
	//inline DriveShaft* GetShaft() { return m_shaft; } 
	//inline float GetWeighLoad() const { return m_suspensionState.weightLoad; }
	inline float GetLateralForce() const
	{
		return m_lateralForce;
	}
	inline float GetLongitudinalForce() const
	{
		return m_longitudinalForce;
	}
	inline float GetSteeringAngle() const
	{
		return m_steeringAngle;
	}

	void Update(float dt);

private:
	void CalcTractionForces(float dt);
	float CalcLateralStoppingImpulse(const Vector3f& axis);
	void ClampTractionForce(float dt);

	friend class Vehicle;

	Vehicle* m_vehicle;
	const VehicleAxleParams* m_axleParams;
	uint32 m_axleIndex;
	uint32 m_sideOfAxle;
	//float m_angle;
	//float m_angularVelocity;
	//float m_inputTorque;

	float m_weightLoad;
	RadiansPerSecond m_speed;
	Radians m_angle;

	// Offset from vehicle body
	Matrix4f m_offset;

	// Is this wheel powered by the motor?
	bool m_powered;

	// The shaft this wheel is attached to, or NULL.
	//DriveShaft* m_shaft;

	//-------------------------------------------------------------------------
	// Tire state parameters

	// Frictional coefficient of the surface.
	float m_adhesionCoefficient;

	// Tire state parameters
	Radians m_steeringAngle;
	Meters m_turningRadius;
	float m_turningSpeedScale;
	float m_brakeAmount;
	NewtonMeters m_brakingTorque;

	// Transformations
	Matrix4f m_wheelToBody;
	Matrix4f m_bodyToWheel;
	Matrix4f m_wheelToWorld;
	Matrix4f m_worldToWheel;
	Vector3f m_groundPosition; // in world space

	// Supsension
	//WheelSuspensionState m_suspensionState;

	// Tire traction state
	float m_slipRatio;
	Radians m_slipAngle;
	float m_longitudinalGrip;
	float m_lateralGrip;
	Newtons m_longitudinalForce;
	Newtons m_lateralForce;
	Newtons m_maxLateralForce;
	Newtons m_maxLongitudinalForce;
	float m_tractionForceScale;
	NewtonMeters m_responseTorque; // Response torque from friction with the ground, sent back to the engine
	float m_skidEnergy;
};


struct VehicleOperatingParams
{
	float throttle;
	float steering;
	float steeringAngle;
	float brake;

	VehicleOperatingParams() :
		throttle(0.0f),
		steering(0.0f),
		steeringAngle(0.0f),
		brake(0.0f)
	{
	}
};


class Vehicle
{
public:
	Vehicle(VehicleParams params = VehicleParams());
	~Vehicle();

	// Getters
	Vector3f GetPosition() const;
	Quaternion GetOrientation() const;
	Vector3f GetVelocity() const;
	Vector3f GetAngularVelocity() const;
	const VehicleParams& GetParams() const;

	const Array<Wheel*>& GetWheels() const
	{
		return m_wheels;
	}
	const size_t GetNumWheels() const
	{
		return m_wheels.size();
	}

	inline void SetOperatingParams(const VehicleOperatingParams& operatingParams)
	{
		m_operatingParams = operatingParams;
	}

	inline const VehicleOperatingParams& GetOperatingParams() const
	{
		return m_operatingParams;
	}


	inline RigidBody* GetBody()
	{
		return m_body;
	}

	void SetPosition(const Vector3f& position);
	void SetOrientation(const Quaternion& orientation);

	void Update(float dt);
	void UpdateSteering(float dt);
	void UpdateEngine(float dt);
	void UpdateBraking(float torqueMultiplier, float dt);
	void UpdateWheels(float dt);

public:
	RigidBody* m_body;
	//Vector3f m_position;
	//Quaternion m_orientation;
	//Vector3f m_velocity;
	//Vector3f m_angularVelocity;
	VehicleParams m_params;
	Radians m_steeringAngle;
	VehicleOperatingParams m_operatingParams;

	//Vector3f m_centerOfMassWorld;

	Wheel m_wheelPool[VEHICLE_MAX_WHEEL_COUNT];
	Array<Wheel*> m_wheels;

private:
	//Matrix4f m_bodyToWorld;
};


#endif // _VEHICLE_H_