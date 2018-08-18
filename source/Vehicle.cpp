#include "Vehicle.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Vehicle::Vehicle(VehicleParams params)
	: m_params(params)
	, m_steeringAngle(0.0f)
{
	m_body = new RigidBody();

	m_body->SetCenterOfMass(m_params.body.centerOfMassOffset);
	m_body->SetMass(m_params.body.mass);
	m_body->SetInverseInertiaTensor(m_params.body.inertiaTensor.GetInverse());

	// Create the wheels for each axle
	for (uint32 i = 0; i < m_params.numAxles; ++i)
	{
		const VehicleAxleParams* axle = m_params.axles + i;

		for (uint32 j = 0; j < axle->numWheels; ++j)
		{
			uint32 wheelIndex = (i * 2) + j;
			Wheel* wheel = &m_wheelPool[wheelIndex];
			m_wheels.push_back(wheel);

			wheel->m_vehicle	= this;
			wheel->m_axleIndex	= i;
			wheel->m_sideOfAxle	= j;
			wheel->m_axleParams	= axle;
			wheel->m_powered = (axle->torqueBias > 0.0f);
						
			// Calculate the offset transformation
			Vector3f wheelOffsetFromAxle = axle->wheelOffset;
			if (j == (int) AxleSide::LEFT && axle->numWheels > 1)
				wheelOffsetFromAxle.x = -wheelOffsetFromAxle.x;
			wheel->m_offset = Matrix4f::CreateTranslation(
				axle->offset + wheelOffsetFromAxle);
			wheel->m_wheelToBody = wheel->m_offset;
			wheel->m_bodyToWheel = Matrix4f::CreateTranslation(
				-axle->offset - wheelOffsetFromAxle);
		}
	}
}

Vehicle::~Vehicle()
{
	delete m_body;
}

void Wheel::Update(float dt)
{
	// Udpate transformations
	m_wheelToBody = m_offset * 
		Matrix4f::CreateRotation(Vector3f::UNITZ, m_steeringAngle);
	m_wheelToWorld = m_vehicle->GetBody()->GetBodyToWorld() * m_wheelToBody;
	m_bodyToWheel = m_wheelToBody;
	m_bodyToWheel.InvertAffine();
	m_worldToWheel = m_bodyToWheel * m_vehicle->GetBody()->GetBodyToWorld();
	
	m_groundPosition = m_wheelToWorld * Vector3f(0.0f, 0.0f, -GetRadius());
	//std::cout << m_groundPosition << std::endl;

	m_angle += m_speed * dt;

	//-------------------------------------------------------------------------
	// Calculate traction forces

	m_tractionForceScale = 1.0f;
	CalcTractionForces(dt);
	
	m_adhesionCoefficient = 1.0f;

	// Calculate default maximum traction force.
	m_weightLoad = 9.81f * m_vehicle->GetParams().body.mass * 0.25f;
	m_maxLateralForce = m_weightLoad *
		m_axleParams->wheels.latTractionMult *
		m_adhesionCoefficient;
	m_maxLongitudinalForce = m_weightLoad *
		m_axleParams->wheels.longTractionMult *
		m_adhesionCoefficient;

	// Modify maximum traction forces for the Pacejka traction model.
	//if (wheel->vehicle->tractionModel == TRACTION_MODEL_PACEJKA)
		//vehicleWheelCalcForcesPacejka(wheel, dt);

	//-------------------------------------------------------------------------
	// Clamp force to the traction ellipse

	m_skidEnergy = 0.0f;
	m_tractionForceScale = 1.0f;

	// Limit the traction based on the maximum friction force.
	// This will calculate a scalar to be multiplied by both longitudinal 
	// and lateral forces to clamp them within the traction ellipse.
	ClampTractionForce(dt);
}


// Calculate the desired traction forces required to stop motion for this wheel.
void Wheel::CalcTractionForces(float dt)
{
	m_lateralForce = 0.0f;
	m_longitudinalForce = 0.0f;
	if (!IsOnGround())
		return;
	
	Vector3f forwardAxis = m_wheelToWorld.c1.xyz;
	Vector3f rightAxis = m_wheelToWorld.c0.xyz;
	float radius = m_axleParams->wheels.radius;

	//DriveTrain* driveTrain = m_vehicle->GetDriveTrain();

	if (dt > 0.0f)
	{
		// Calculate the force required to completely remove
		// all slip velocity within one frame.
		Vector3f groundVelocity =
			m_vehicle->GetBody()->GetVelocityAtPoint(m_groundPosition);
		float tirePatchSpeed = m_speed * radius;
		float groundSpeed = groundVelocity.Dot(forwardAxis);
		float slipVelocity = tirePatchSpeed - groundSpeed;
		//unsigned int numLockedWheels =
			//driveTrain->CountLockedWheelsBackward(m_input);
		//float inertiaInv = ((float) numLockedWheels) /
			//driveTrain->BackwardCalcInertia(this);
		float inertia = 50.0f;
		float inertiaInv = 1.0f / inertia;

		float massInv = m_vehicle->GetBody()->GetInverseMass();
		float longImpulse = slipVelocity / ((inertiaInv * radius * radius) -
			massInv);
		//float longImpulse = slipVelocity * m_vehicle->GetBody()->GetMass() * 0.1f;
		//if (m_powered)
		//	m_longitudinalForce = longImpulse / dt;
		//else
			m_longitudinalForce = 0.0f;

		// Calculate the force required to completely stop
		// lateral motion within one frame.
		float latImpulse = CalcLateralStoppingImpulse(rightAxis);
		m_lateralForce = latImpulse / dt;
		m_lateralForce /= (float) m_vehicle->GetNumWheels();

		// Calculate the force required to completely remove
		//// all slip velocity within one frame.
		//Vector3f groundVelocity =
		//	m_vehicle->GetBody()->GetVelocityAtPoint(m_groundPosition);
		//float tirePatchSpeed = m_input->GetSpeed() * radius;
		//float groundSpeed = groundVelocity.Dot(forwardAxis);
		//float slipVelocity = tirePatchSpeed - groundSpeed;
		//unsigned int numLockedWheels =
		//	driveTrain->CountLockedWheelsBackward(m_input);
		//float inertiaInv = ((float) numLockedWheels) /
		//	driveTrain->BackwardCalcInertia(this);
		//
		//float longImpulse = slipVelocity / ((inertiaInv * radius * radius) -
		//	m_vehicle->GetBody()->GetInverseMass());
		//m_longitudinalForce = longImpulse / dt;
		//
		//// Calculate the force required to completely stop
		//// lateral motion within one frame.
		//float latImpulse = CalcLateralStoppingImpulse(rightAxis);
		//m_lateralForce = latImpulse / dt;
		//m_lateralForce /= (float) m_vehicle->GetNumWheels();
	}
}

// Calculate the impulse required to stop all lateral motion.
// Idea taken and modified from Digital Rune:
// https://github.com/DigitalRune/DigitalRune/blob/master/Source/DigitalRune.Physics.Specialized/Vehicles/VehicleForceEffect.cs
float Wheel::CalcLateralStoppingImpulse(const Vector3f& direction)
{
	RigidBody* body = m_vehicle->GetBody();
	Vector3f radiusVector = m_groundPosition - body->GetCenterOfMassWorld();
	Vector3f contactVelocity = body->GetVelocityAtPoint(m_groundPosition);

	float velocityChangePerUnitImpulse = 0.0f;
	Vector3f deltaVelWorld = Vector3f::Cross(
		m_vehicle->GetBody()->GetInverseInertiaTensorWorld() *
		radiusVector.Cross(direction), radiusVector);
	velocityChangePerUnitImpulse += deltaVelWorld.Dot(direction);
	velocityChangePerUnitImpulse += body->GetInverseMass();
	
	// Calculate the desired contact velocity after applying the impulse.
	float desiredDeltaVelocity = -contactVelocity.Dot(direction);

	// Now calculate the impulse.
	// This acts in the opposite direction of the contact normal.
	float impulseScalar = desiredDeltaVelocity / velocityChangePerUnitImpulse;
	return impulseScalar;
}

void Wheel::ClampTractionForce(float dt)
{
	m_tractionForceScale = 1.0f;
	m_skidEnergy = 0.0f;
	
	m_adhesionCoefficient = 1.0f;
	m_maxLateralForce = m_weightLoad *
		m_axleParams->wheels.latTractionMult *
		m_adhesionCoefficient;
	m_maxLongitudinalForce = m_weightLoad *
		m_axleParams->wheels.longTractionMult *
		m_adhesionCoefficient;

	if (!IsOnGround() || m_weightLoad < 0.0001f)
	{
		m_longitudinalForce = 0.0f;
		m_lateralForce = 0.0f;
		return;
	}

	// Clamp the traction force to an ellipse shape.
	Vector2f totalForce(m_lateralForce, m_longitudinalForce);
	Vector2f maxForce(m_maxLateralForce, m_maxLongitudinalForce);

	if (maxForce.LengthSquared() == 0.0f)
	{
		m_longitudinalForce = 0.0f;
		m_lateralForce = 0.0f;
		m_tractionForceScale = 0.0f;
		return;
	}

	float totalForceRadius = 
		((totalForce.x * totalForce.x) / (maxForce.x * maxForce.x)) +
		((totalForce.y * totalForce.y) / (maxForce.y * maxForce.y));
	if (totalForceRadius <= 1.0f)
		return;

	float denom = sqrtf(
		(maxForce.y * maxForce.y * totalForce.x * totalForce.x) +
		(maxForce.x * maxForce.x * totalForce.y * totalForce.y));
	if (denom == 0.0f)
		return;

	m_tractionForceScale = (maxForce.x * maxForce.y) / denom;
	if (m_tractionForceScale >= 1.0f)
		return;

	// Calculate skid energy.
	float skidForce = totalForce.Length() * (1.0f - m_tractionForceScale);
	float skidVelocity = (skidForce / m_vehicle->GetBody()->GetMass()) * dt;
	m_skidEnergy = 2000.0f * skidVelocity * dt;
}





//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

Vector3f Vehicle::GetPosition() const
{
	return m_body->GetPosition();
}

Quaternion Vehicle::GetOrientation() const
{
	return m_body->GetOrientation();
}

Vector3f Vehicle::GetVelocity() const
{
	return m_body->GetVelocity();
}

Vector3f Vehicle::GetAngularVelocity() const
{
	return m_body->GetAngularVelocity();
}

const VehicleParams& Vehicle::GetParams() const
{
	return m_params;
}



//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void Vehicle::SetPosition(const Vector3f& position)
{
	m_body->SetPosition(position);
}

void Vehicle::SetOrientation(const Quaternion& orientation)
{
	m_body->SetOrientation(orientation);
}


//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------

void Vehicle::Update(float dt)
{
	//m_body->m_angularVelocity *= 0.75f;
	// Udpate transformations
	m_body->CalculateDerivedData();

	UpdateSteering(dt);
	UpdateWheels(dt);

	//m_body->IntegrateAngular(dt);
	//m_body->IntegrateLinear(dt);

	//m_body->ClearAccumulators();
}

void Vehicle::UpdateSteering(float dt)
{
	uint32 i;
	
	//-------------------------------------------------------------------------
	// Update steering angle control
	
	const VehicleSteeringParams* params = &m_params.steering;

	// Determine the amount to interpolate between "slow" and "fast"
	// steering rates.
	float vehicleSpeed = GetBody()->GetVelocity().Length();
	float fastFactor = 0.0f;
	if (params->speedFast != params->speedSlow)
	{
		fastFactor = (vehicleSpeed - params->speedSlow) /
			(params->speedFast - params->speedSlow);
		fastFactor = Math::Clamp(fastFactor, 0.0f, 1.0f);
	}

	// Get the steering rates for the current vehicle speed.
	float maxSteeringAngle = Math::Lerp(params->maxAngleSlow,
		params->maxAngleFast, fastFactor);
	float turnRate = Math::Lerp(params->turnRateSlow,
		params->turnRateFast, fastFactor);
	float alignRate = Math::Lerp(params->alignRateSlow,
		params->alignRateFast, fastFactor);

	// Apply steering input.
	//if (m_operatingParams.steering != 0.0f)
	//{
	//	// Turn steering.
	//	m_steeringAngle += m_operatingParams.steering * turnRate * dt;
	//}
	//else
	//{
	//	// Align steering back to straight.
	//	if (Math::Abs(m_steeringAngle) > 0.01f)
	//	{
	//		m_steeringAngle -= Math::Sign(m_steeringAngle) *
	//			Math::Min(alignRate * dt, Math::Abs(m_steeringAngle));
	//	}
	//	else
	//		m_steeringAngle = 0.0f;
	//}

	// Clamp steering angle.
	m_steeringAngle = params->maxAngleSlow *
		Math::Clamp(m_operatingParams.steeringAngle, -1.0f, 1.0f);

	//-------------------------------------------------------------------------
	// Apply unmodified steering angles to the wheels.
	
	for (Wheel* wheel : m_wheels)
	{
		wheel->m_steeringAngle = wheel->m_axleParams->steeringAngleMult *
			m_steeringAngle;
		wheel->m_turningSpeedScale = 1.0f;
		wheel->m_turningRadius = 1.0f;
	}
	
	//-------------------------------------------------------------------------
	// Apply Ackermann steering geometry to adjust the steering angles.

	// Determine the front and rear axles.
	int32 frontAxleIndex = -1;
	int32 rearAxleIndex = -1;
	Radians minAxleForward = FLT_MAX;
	Radians maxAxleForward = -FLT_MIN;
	for (i = 0; i < m_params.numAxles; ++i)
	{
		float forward = -m_params.axles[i].offset.y -
			m_params.axles[i].wheelOffset.y;
		if (forward > maxAxleForward)
		{
			frontAxleIndex = i;
			maxAxleForward = forward;
		}
		if (forward < minAxleForward)
		{
			rearAxleIndex = i;
			minAxleForward = forward;
		}
	}

	// Determine the steering angles of the front and rear axles.
	float frontAngle = m_steeringAngle *
		m_params.axles[frontAxleIndex].steeringAngleMult;
	float rearAngle = -m_steeringAngle *
		m_params.axles[rearAxleIndex].steeringAngleMult;
	VehicleAxleParams* frontAxle = m_params.axles + frontAxleIndex;
	VehicleAxleParams* rearAxle = m_params.axles + rearAxleIndex;
	
	// Intersect lines perpendicular to the front and rear axle's steering
	// angles to obtain the turn center point (if the lines are parallel,
	// then the vehicle is not turning).
	Vector2f frontLateralDir =
		Vector2f(Math::Cos(frontAngle), Math::Sin(frontAngle));
	Vector2f rearLateralDir =
		Vector2f(Math::Cos(rearAngle), Math::Sin(rearAngle));
	if (Math::Abs(Vector2f::Dot(frontLateralDir, rearLateralDir)) < 0.999f)
	{
		// Intersect the lines to find the turn center point.
		Vector2f frontAxlePoint(frontAxle->offset.x, -frontAxle->offset.y);
		Vector2f rearAxlePoint(rearAxle->offset.x, -rearAxle->offset.y);
		Vector2f turnCenter = Line2f::GetLineIntersection(
			frontAxlePoint, frontAxlePoint + frontLateralDir,
			rearAxlePoint, rearAxlePoint + rearLateralDir);
		
		// Now apply ackermann geometry to the steered wheels,
		// adjusting their steering angles.
		for (Wheel* wheel : m_wheels)
		{
			Vector2f wheelToTurnCenter = turnCenter - 
				Vector2f(wheel->m_offset.c3.x, -wheel->m_offset.c3.y);
			wheel->m_turningRadius = wheelToTurnCenter.Length();

			if (wheel->m_axleParams->ackermannGeometry)
			{
				wheel->m_steeringAngle = Math::ATan(
					wheelToTurnCenter.y / -wheelToTurnCenter.x);
			}
		}
	}

	//-------------------------------------------------------------------------
	// Calculate speed multipliers for powered wheels based on the ratio of
	// their turning radii (this is essentially faked differentials at work).

	// NOTE: This is no longer used now that the drive-train is represented
	// through connected shafts and differential constraints. These speed
	// multipliers could still be used if each wheel has its own motor.

	Wheel* firstPoweredWheel = NULL;
	unsigned int numPoweredWheels = 0;
	float radiusRatioSum = 0.0f;
	
	// Find the first powered wheel and also calculate the sum 
	// of the turning radius ratios of all other wheels.
	for (Wheel* wheel : m_wheels)
	{
		if (wheel->m_powered)
		{
			numPoweredWheels++;

			if (firstPoweredWheel == NULL)
			{
				firstPoweredWheel = wheel;
			}
			else
			{
				radiusRatioSum += wheel->m_turningRadius /
					wheel->m_axleParams->wheels.radius;
			}
		}
	}
	
	// Calculate multiplier for the first powered wheel.
	firstPoweredWheel->m_turningSpeedScale = numPoweredWheels / (1.0f +
		(firstPoweredWheel->m_axleParams->wheels.radius /
		firstPoweredWheel->m_turningRadius) * radiusRatioSum);

	// Now calculate the turning speed multipliers for all other powered wheels.
	for (Wheel* wheel : m_wheels)
	{
		if (wheel->m_powered && wheel != firstPoweredWheel)
		{
			wheel->m_turningSpeedScale = (firstPoweredWheel->m_turningSpeedScale *
				firstPoweredWheel->m_axleParams->wheels.radius * wheel->m_turningRadius) /
				(firstPoweredWheel->m_turningRadius * wheel->m_axleParams->wheels.radius);
		}
	}
}

void Vehicle::UpdateWheels(float dt)
{
	for (Wheel* wheel : m_wheels)
	{
		if (wheel->m_powered)
		{
			wheel->m_speed += (m_operatingParams.throttle * Math::ToRadians(3600.0f)) * dt;
		}
		
		wheel->Update(dt);
		
		wheel->m_lateralForce *= wheel->m_tractionForceScale;
		wheel->m_longitudinalForce *= wheel->m_tractionForceScale;
		//if (wheel != m_wheels[0])
		//	wheel->m_lateralForce = 0.0f;
		//else
		//	wheel->m_lateralForce *= 4.0f;

		// Get the traction force from the tire
		Vector3f tractionForce = Vector3f::ZERO;
		tractionForce.x = wheel->m_lateralForce;
		tractionForce.y = wheel->m_longitudinalForce;

		// Calculate road-response torque
		//wheel->m_responseTorque = -wheel->m_longitudinalForce *
			//wheel->m_axleParams->wheels.radius;

		// Transform traction force into vehicle space
		tractionForce = wheel->m_wheelToBody.Rotate(tractionForce);
		//m_tractionForce += tractionForce;

		// Calculate the traction torque in vehicle space.
		// Apply pitch and roll reduction to this torque to limit rotation.
		Vector3f momentArm = wheel->m_wheelToBody.c3.xyz - m_body->GetCenterOfMass();
		//Vector3f tractionTorque = momentArm.Cross(tractionForce);
		//tractionTorque.x *= (1.0f - m_params.body.pitchReduction);
		//tractionTorque.y *= (1.0f - m_params.body.rollReduction);
		//tractionTorque.x = 0.0f;
		//tractionTorque.y = 0.0f;

		// Transform traction force and torque into world space and apply them.
		//tractionForce = wheel->m_wheelToWorld.Rotate(tractionForce);
		tractionForce = m_body->GetBodyToWorld().Rotate(tractionForce);
		//tractionTorque = m_body->GetBodyToWorld().Rotate(tractionTorque);
		//m_body->ApplyForceLinear(tractionForce);
		m_body->ApplyForce(tractionForce, wheel->m_groundPosition);
		//m_body->ApplyTorque(tractionTorque);
		m_body->m_torque.xy = Vector2f::ZERO;
		m_body->SetAngularVelocity(m_body->GetAngularVelocity() * Vector3f(0.0f,1.0f,1.0f));

		//float inertiaInv = 1.0f / 50.0f;
		//wheel->m_responseTorque = -wheel->m_longitudinalForce *
		//	wheel->m_axleParams->wheels.radius;
		//wheel->m_speed += wheel->m_responseTorque * inertiaInv * dt;
	}
}

