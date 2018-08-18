#ifndef _VEHICLE_PARAMS_H_
#define _VEHICLE_PARAMS_H_

#include "CommonTypes.h"
#include <cmgGraphics/cmg_graphics.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

#define VEHICLE_MAX_AXLE_COUNT		4
#define VEHICLE_MAX_GEAR_COUNT		8
#define VEHICLE_MAX_WHEEL_COUNT		(2 * VEHICLE_MAX_AXLE_COUNT)


//-----------------------------------------------------------------------------
// VehicleWheelParams
//-----------------------------------------------------------------------------
struct VehicleWheelParams
{
	Meters radius;
	Kilograms mass;
	float inertia;
	
	//PacejkaSimple longitudinalPacejka;
	//PacejkaSimple lateralPacejka;

	// multiplier for maximum lateral traction force
	float latTractionMult;

	// multiplier for maximum longitudinal traction force
	float longTractionMult;

	// rolling resistance coefficient
	float rollingResistance;
};


//-----------------------------------------------------------------------------
// VehicleSuspensionParams
//-----------------------------------------------------------------------------
struct VehicleSuspensionParams
{
	// Sprint constants
	float springStiffness;
	float springDamping;
	
	// Spring lengths
	float restLength;
	float maxLength;
	float minLength;
};


//-----------------------------------------------------------------------------
// VehicleAxleParams
//-----------------------------------------------------------------------------

struct VehicleAxleParams
{	
	// Center point of this axle in vehicle object space.
	Vector3f offset;

	// Offset of wheel position from the axle's center point in vehicle
	// object space. This assumes that both wheels are symmetric.
	// NOTE: the wheel track for this axle = (2 * wheelOffset.x).
	Vector3f wheelOffset;

	// An axle can have either 1 or 2 wheels.
	uint32 numWheels;

	// Parameters shared between the wheels on this axle.
	VehicleWheelParams wheels;

	// Suspension parameters for the wheels on this axle.
	VehicleSuspensionParams suspension;

	// How much torque does this axle get compared to other axles.
	// This value is normalized to 1 across all axles.
	float torqueBias;
	
	// How much braking torque does this axle get compared to other axles.
	// This value is normalized to 1 across all axles.
	float brakeBias;
	
	// Multiplies the vehicle's steering angle.
	float steeringAngleMult;

	// Should Ackermann steering geometry be applied to this wheel,
	// modifying its steering angle?
	bool ackermannGeometry;
};


//-----------------------------------------------------------------------------
// VehicleSteeringParams
//-----------------------------------------------------------------------------
struct VehicleSteeringParams
{
	// The min and max vehicle speeds that define "slow" and "fast"
	MetersPerSecond speedSlow;
	MetersPerSecond speedFast;

	// Maximum steering angle
	Radians maxAngleSlow;
	Radians maxAngleFast;
	
	// Rate at which the steering wheel is turned by the driver.
	RadiansPerSecond turnRateSlow;
	RadiansPerSecond turnRateFast;

	// Rate at which the steering wheel is aligned to it's resting state.
	float alignRateSlow;
	float alignRateFast;

	// This makes the steering response non-linear.
	// The steering function is linear, then raised to this power
	float steeringExponent;
};


//-----------------------------------------------------------------------------
// VehicleBodyParams
//-----------------------------------------------------------------------------
struct VehicleBodyParams
{
	Kilograms mass;

	Matrix3f inertiaTensor;

	Vector3f size;

	Vector3f centerOfMassOffset;

	// Measured value. This incorperates frontal area.
	float dragCoefficient;

	// How much rolling torque is reduced from traction forces.
	// 0 = unmidified roll, 1 = no roll.
	float rollReduction;
	
	// How much pitching torque is reduced from traction forces.
	// 0 = unmidified pitch, 1 = no pitch.
	float pitchReduction;
};



//-----------------------------------------------------------------------------
// VehicleGraphicsParams
//-----------------------------------------------------------------------------
struct VehicleGraphicsParams
{
	Vector3f driverHeadPostion;

	Vector3f steeringWheelAttachmentPoint;
	Vector3f steeringWheelDirection;
	
	Mesh* meshWheel;
	Mesh* meshBody;
};

//-----------------------------------------------------------------------------
// VehicleParams
//-----------------------------------------------------------------------------
struct VehicleParams
{
	uint32 numAxles;
	VehicleBodyParams body;
	VehicleAxleParams axles[VEHICLE_MAX_AXLE_COUNT];
	//VehicleEngineParams engine;
	//VehicleTransmissionParams transmission;
	VehicleSteeringParams steering;
	//VehicleBrakesParams brakes;
	VehicleGraphicsParams graphics;


	// Visuals
	Mesh* meshWheel;
	Mesh* meshBody;

	VehicleParams()
	{
		graphics.meshBody = nullptr;
		graphics.meshWheel = nullptr;
	}
};


#endif // _VEHICLE_PARAMS_H_