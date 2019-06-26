#include "DrivingApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>
#include <fstream>

#define ASSETS_PATH "C:/workspace/c++/cmg/RoadMind/assets/"


// calculate local inertia tensors for different shapes
Matrix3f createInertiaTensorBox(float mass, const Vector3f& halfBox)
{
	if (mass > 0.0f)
	{
		float k = (1.0f / 12.0f) * mass * 4.0f; // multiply by 4.0 because sizes are half-dimensions
		float xx = (float)halfBox.x * (float)halfBox.x;
		float yy = (float)halfBox.y * (float)halfBox.y;
		float zz = (float)halfBox.z * (float)halfBox.z;
		return Matrix3f::CreateScale(k * (yy + zz), k * (zz + xx), k * (xx + yy));
	}
	return Matrix3f::IDENTITY;
}

DrivingApp::DrivingApp()
{
}

DrivingApp::~DrivingApp()
{
}


//-----------------------------------------------------------------------------
// Overridden Methods
//-----------------------------------------------------------------------------

void DrivingApp::OnInitialize()
{
	m_font = SpriteFont::LoadBuiltInFont(BuiltInFonts::FONT_CONSOLE);
	m_debugDraw = new DebugDraw();

	m_defaultCameraState.m_viewHeight = 50.0f;
	m_defaultCameraState.m_position = Vector2f::ZERO;
	m_defaultCameraState.m_rotation = 0.0f;
	m_defaultCameraState.m_aspectRatio = GetWindow()->GetAspectRatio();
	m_camera = m_defaultCameraState;

	m_wheel = GetInputManager()->AddDevice<Joystick>();

	Mesh::Load(ASSETS_PATH "toyota_ae86.obj", m_vehicleMesh);
	Mesh::Load(ASSETS_PATH "wheel.obj", m_meshWheel);


	//-------------------------------------------------------------------------
	// Custom Toyota Corolla AE85 low poly
	//-------------------------------------------------------------------------

	VehicleParams params;

	// Body
	params.body.size.x = 0.5f;
	params.body.size.y = 0.5f;
	params.body.size.z = 0.5f;
	params.body.mass = 1435;
	params.body.inertiaTensor = createInertiaTensorBox(params.body.mass, params.body.size);
	params.body.centerOfMassOffset = Vector3f(0.0f, 0.33216f, -0.30f);
	params.body.dragCoefficient = 0.39f;
	params.body.rollReduction = 0.4f;
	params.body.pitchReduction = 0.0f;
	params.graphics.driverHeadPostion = Vector3f(-0.377f, 0.902f, 0.073f);

	//-------------------------------------------------------------------------
	// Engine: M30B35 6-cyl SOHC

	//params.engine.idleSpeed					= rpmToRadPerSec(700);
	//params.engine.revLimiterSpeed			= rpmToRadPerSec(5700);
	//params.engine.torqueCurve.peakTorque	= torqueCurveMaxTorque(305, 4000); // 305 Nm @ 4000 rpm
	//params.engine.torqueCurve.redLine		= torqueCurveMaxPower(155, 5700); // 155 kW @ 5700 rpm

	//-------------------------------------------------------------------------
	// Transmission

	//params.transmission.numGears			= 5;
	//params.transmission.gearRatios[0]		= 3.17f;
	//params.transmission.gearRatios[1]		= 1.88f;
	//params.transmission.gearRatios[2]		= 1.30f;
	//params.transmission.gearRatios[3]		= 1.00f;
	//params.transmission.gearRatios[4]		= 0.90f;
	//params.transmission.reverseGearRatio	= 4.00f;
	//params.transmission.finalDriveRatio		= 4.44f;
	//params.transmission.efficiency			= 0.90f;
	//params.transmission.isAutoTransmission	= 1;
	//params.transmission.shiftDownSpeed		= rpmToRadPerSec(3000);
	//params.transmission.shiftUpSpeed		= rpmToRadPerSec(5500);

	//-------------------------------------------------------------------------
	// Brakes

	//params.brakes.brakingTorque	= 1000 * 4.0f;

	//-------------------------------------------------------------------------
	// Axles & Wheels

	params.numAxles = 2;

	// Front axle
	params.axles[0].offset = Vector3f(0.0f, 1.25158f, -0.33f);
	params.axles[0].wheelOffset = Vector3f(0.71697f, 0.0f, 0.0f);
	params.axles[0].torqueBias = 0.0f;
	params.axles[0].brakeBias = 0.25f;
	params.axles[0].steeringAngleMult = 1.0f;
	params.axles[0].ackermannGeometry = true;
	params.axles[0].numWheels = 2;
	params.axles[0].wheels.radius = 0.310f;
	params.axles[0].wheels.mass = 13.0f;
	params.axles[0].wheels.inertia = 0.0f; // TODO
										   //params.axles[0].wheels.latTractionMult = 2.50f;
										   //params.axles[0].wheels.longTractionMult = 2.50f;
	params.axles[0].wheels.latTractionMult = 1.0f;
	params.axles[0].wheels.longTractionMult = 1.0f;
	params.axles[0].wheels.rollingResistance = 0.02f;
	//params.axles[0].suspension.springStiffness	= 100000000.0f;
	//params.axles[0].suspension.springDamping	= 35000;
	params.axles[0].suspension.springStiffness = 69686;
	params.axles[0].suspension.springDamping = 70;
	//params.axles[0].suspension.springStiffness	= 100;
	//params.axles[0].suspension.springDamping	= 0.0f;
	params.axles[0].suspension.restLength = 0.310f * 1.5f;
	params.axles[0].suspension.minLength = 0.310f;
	params.axles[0].suspension.maxLength = 0.310f * 1.5f;
	//params.axles[0].wheels.longitudinalPacejka	= pacejkaSimpleCreate(10.0f, 1.6f, 1.0f, 0.8f);
	//params.axles[0].wheels.longitudinalPacejka	= pacejkaSimpleCreate(10.0f, 2.1f, 1.0f, 0.6f);
	//params.axles[0].wheels.lateralPacejka		= pacejkaSimpleCreate(4.0f, 2.0f, 1.0f, -0.1f);
	params.axles[0].wheels.latTractionMult = 1.80f;
	params.axles[0].wheels.longTractionMult = 2.00f;

	// Rear axle
	params.axles[1] = params.axles[0];
	params.axles[1].offset.y = -1.22317f;
	params.axles[1].torqueBias = 1.0f;
	params.axles[1].brakeBias = 0.75f;
	params.axles[1].steeringAngleMult = 0.0f;
	params.axles[1].ackermannGeometry = false;

	// Steering
	params.steering.speedSlow = mphToMetersPerSecond(5);
	params.steering.speedFast = mphToMetersPerSecond(55);
	params.steering.maxAngleSlow = Math::ToRadians(40.0f);
	params.steering.maxAngleFast = Math::ToRadians(4.0f);
	params.steering.turnRateSlow = Math::ToRadians(80.0f);
	params.steering.turnRateFast = Math::ToRadians(8.0f);
	params.steering.alignRateSlow = Math::ToRadians(80.0f);
	params.steering.alignRateFast = Math::ToRadians(8.0f);
	params.steering.steeringExponent = 1.0f;

	// Graphics
	params.graphics.meshBody = m_vehicleMesh;
	params.graphics.meshWheel = m_meshWheel;
	params.graphics.driverHeadPostion = Vector3f(-0.29761f, 0.00362f, 0.46259f);

	m_vehicleParams = params;

	Array<uint8> fileData;
	File::OpenAndGetContents("C:/files/scripts/germany.tlm", fileData);
	uint32 count = fileData.size() / sizeof(SimulationData);
	m_simulationData.resize(count);
	memcpy(m_simulationData.data(), fileData.data(), count * sizeof(SimulationData));
	m_lapTime = m_simulationData[0].time;

	Reset();
}


void DrivingApp::OnQuit()
{
	delete m_font;
	m_font = nullptr;
	delete m_player;
	m_player = nullptr;
}

void DrivingApp::OnUpdate(float dt)
{
	Mouse* mouse = GetMouse();
	Keyboard* keyboard = GetKeyboard();
	Window* window = GetWindow();
	MouseState mouseState = mouse->GetMouseState();
	Vector2f windowSize((float) window->GetWidth(), (float) window->GetHeight());
	Vector2f mousePosition;
	mousePosition.x = (float) mouseState.x;
	mousePosition.y = (float) mouseState.y;

	bool ctrl = (keyboard->IsKeyDown(Keys::left_control) ||
		keyboard->IsKeyDown(Keys::right_control));
	bool shift = (keyboard->IsKeyDown(Keys::left_shift) ||
		keyboard->IsKeyDown(Keys::right_shift));
	int scroll = mouseState.z - mouse->GetPrevMouseState().z;

	// ESCAPE: Quit
	if (keyboard->IsKeyPressed(Keys::escape))
	{
		Quit();
		return;
	}

	//UpdateVehicleControls(dt);

	m_lapTime += dt;
	VehicleOperatingParams operatingParams;
	//orientation = Quaternion::IDENTITY;
	for (uint32 i = 0; i < m_simulationData.size(); i++)
	{
		if (m_simulationData[i].time > m_lapTime)
		{
			auto& data = m_simulationData[i];
			Vector3f v = data.position - m_simulationData[0].position;
			m_player->SetPosition(Vector3f(v.GetXZ(), v.y));
			operatingParams.throttle = data.throttle;
			operatingParams.steeringAngle = data.steering;
			operatingParams.brake = data.brake;
			m_rollVector.x = data.roll.x;
			m_rollVector.y = data.roll.z;
			m_rollVector.z = data.roll.y;
			m_pitchVector.x = data.pitch.x;
			m_pitchVector.y = data.pitch.z;
			m_pitchVector.z = data.pitch.y;
			Vector3f forward = m_pitchVector;
			Vector3f left = m_rollVector;
			Vector3f up = forward.Cross(left);
			up.Normalize();
			m_upVector = up;
			m_leftVector = left;
			m_forwardVector = forward;
			Matrix3f m(-left, forward, up);
			m_player->SetOrientation(m.ToQuaternion());
			break;
		}
	}
	m_player->SetOperatingParams(operatingParams);

	// Update vehicles
	// m_physicsEngine->Simulate(dt);
	// m_player->Update(dt);

	m_cameraPosition = m_player->GetPosition();
}

void DrivingApp::Reset()
{
	if (m_player != nullptr)
		delete m_player;
	m_player = new Vehicle(m_vehicleParams);
	m_player->SetPosition(Vector3f::ZERO);
	m_physicsEngine = new PhysicsEngine();
	m_physicsEngine->SetGravity(Vector3f::ZERO);
	m_physicsEngine->AddBody(m_player->GetBody());

	m_camera.m_position = m_defaultCameraState.m_position;
	m_camera.m_rotation = m_defaultCameraState.m_rotation;

	// Setup the initial camera.
	m_cameraPosition = Vector3f(0.0f, 0.0f, 0.5f);
	m_cameraYaw = 0.0f;
	m_cameraPitch = Math::HALF_PI * 0.4f;
	m_cameraDistance = 7.0f;

	m_newCamera = Camera();
	m_newCamera.SetPosition(Vector3f::ZERO);
	m_newCamera.SetPerspective(
	GetWindow()->GetAspectRatio(), 1.4f, 0.1f, 1000.0f);
}


void DrivingApp::UpdateCameraControls(float dt)
{
	Mouse* mouse = GetMouse();
	Keyboard* keyboard = GetKeyboard();
	Window* window = GetWindow();
	Vector2f windowSize((float)window->GetWidth(), (float)window->GetHeight());
	Vector2f mousePos((float)mouse->GetMouseState().x, (float)mouse->GetMouseState().y);
	Vector2f mousePosPrev((float)mouse->GetPrevMouseState().x, (float)mouse->GetPrevMouseState().y);
	int scroll = mouse->GetMouseState().z - mouse->GetPrevMouseState().z;
	bool ctrl = keyboard->IsKeyDown(Keys::left_control) ||
		keyboard->IsKeyDown(Keys::right_control);

	Vector3f right = m_newCamera.GetOrientation().GetRight();
	Vector3f forward = Vector3f::Cross(Vector3f::UNITZ, right);

	// Scoll Wheel: Zoom in/out
	if (scroll != 0)
	{
		m_camera.m_viewHeight *= Math::Pow(0.9f, (float)scroll);
		m_cameraDistance *= Math::Pow(0.9f, (float)scroll);
	}

	// WASD: Move camera
	if (!ctrl)
	{
		Vector3f move = Vector3f::ZERO;
		if (keyboard->IsKeyDown(Keys::a))
			move.x -= 1.0f;
		if (keyboard->IsKeyDown(Keys::d))
			move.x += 1.0f;
		if (keyboard->IsKeyDown(Keys::s))
			move.y -= 1.0f;
		if (keyboard->IsKeyDown(Keys::w))
			move.y += 1.0f;
		if (keyboard->IsKeyDown(Keys::e))
			move.z += 1.0f;
		if (keyboard->IsKeyDown(Keys::q))
			move.z -= 1.0f;
		if (move.LengthSquared() > 0.0f)
		{
			move.xy.Normalize();
			m_cameraPosition += right * move.x;
			m_cameraPosition += forward * move.y;
			m_cameraPosition.z += move.z;
			m_cameraPosition.z = Math::Max(m_cameraPosition.z, 0.0f);
		}
	}

	// Ctrl+RMB: Rotate camera
	if (ctrl && mouse->IsButtonDown(MouseButtons::right))
	{
		Vector2f windowCenter = windowSize * 0.5f;
		Vector2f angle = (mousePos - mousePosPrev) * 0.003f;
		m_cameraPitch += angle.y;
		m_cameraPitch = Math::Clamp(m_cameraPitch, 0.0f, Math::HALF_PI);
		m_cameraYaw -= angle.x;
		if (m_cameraYaw > Math::TWO_PI)
			m_cameraYaw -= Math::TWO_PI;
		if (m_cameraYaw < 0.0f)
			m_cameraYaw += Math::TWO_PI;
	}

	if (m_joystick != nullptr)
	{
		auto xbox = m_joystick->GetState();
		float speed = 2.0f;
		if (xbox.rightStick.Length() < 0.25f)
			xbox.rightStick = Vector2f::ZERO;
		m_cameraPitch -= xbox.rightStick.y * speed * dt;
		m_cameraYaw -= xbox.rightStick.x * speed * dt;
	}

	m_newCamera.SetOrientation(
		Quaternion(Vector3f::UNITZ, m_cameraYaw) *
		Quaternion(Vector3f::UNITX, Math::HALF_PI - m_cameraPitch));
	m_newCamera.SetPosition(m_cameraPosition +
		m_newCamera.GetOrientation().GetBack() * m_cameraDistance);
}

void DrivingApp::UpdateVehicleControls(float dt)
{
	Keyboard* keyboard = GetKeyboard();
	Vehicle* vehicle = m_player;
	VehicleOperatingParams operatingParams;

	operatingParams.brake = 0.0f;
	if (keyboard->IsKeyDown(Keys::space))
		operatingParams.brake = 1.0f;
	operatingParams.throttle = 0.0f;
	if (keyboard->IsKeyDown(Keys::up))
		operatingParams.throttle += 1.0f;
	if (keyboard->IsKeyDown(Keys::down))
		operatingParams.throttle -= 1.0f;

	operatingParams.steering = 0.0f;
	if (keyboard->IsKeyDown(Keys::left))
		operatingParams.steering += 1.0f;
	if (keyboard->IsKeyDown(Keys::right))
		operatingParams.steering -= 1.0f;

	if (m_joystick != nullptr)
	{
		auto xbox = m_joystick->GetState();
		if (xbox.leftStick.Length() < 0.25f)
			xbox.leftStick = Vector2f::ZERO;
		operatingParams.steeringAngle -= xbox.leftStick.x;
		operatingParams.throttle += xbox.rightTrigger - xbox.leftTrigger;
		operatingParams.brake += xbox.a ? 1.0f : 0.0f;
	}

	if (m_wheel != nullptr)
	{
		operatingParams.steeringAngle -= m_wheel->m_axisPool[0].x;
		operatingParams.throttle += 1.0f - (m_wheel->m_axisPool[1].x + 1.0f) * 0.5f;
		operatingParams.brake += 1.0f - (m_wheel->m_axisPool[5].x + 1.0f) * 0.5f;
	}

	vehicle->GetBody()->ApplyForceLinear(vehicle->GetBody()->GetBodyToWorld().c1.xyz *
		vehicle->GetBody()->GetMass() * 9.81f * operatingParams.throttle * 0.5f);
	//operatingParams.throttle = 0.0f;

	vehicle->SetOperatingParams(operatingParams);

	m_cameraPosition = vehicle->GetPosition();
}

static void DrawArrowHead(Graphics2D& g, const Vector2f& position, const Vector2f& direction, float radius, const Color& color)
{
	Radians angle = Math::PI * 0.25f;
	Vector2f end1 = -direction;
	end1 = end1.Rotate(angle);
	Vector2f end2 = -direction;
	end2 = end2.Rotate(-angle);
	g.DrawLine(position, position + (end1 * radius), color);
	g.DrawLine(position, position + (end2 * radius), color);
}


void DrivingApp::DrawVehicle(Graphics2D& g, Vehicle* vehicle)
{
	Vector3f pos = vehicle->GetPosition();
	Quaternion rot = vehicle->GetOrientation();
	VehicleParams params = vehicle->GetParams();
	rot.Rotate(Vector3f::UNITZ, 0.2f);

	Vector3f forward = Vector3f::UNITY;
	Vector3f right = Vector3f::UNITX;
	Vector3f up = Vector3f::UNITZ;
	rot.RotateVector(forward);
	rot.RotateVector(right);
	rot.RotateVector(up);

	Mesh* mesh = m_vehicleMesh;
	Mesh* meshWheel = m_meshWheel;

	Matrix4f viewProjection = m_newCamera.GetViewProjectionMatrix();
	m_debugDraw->SetViewProjection(viewProjection);
	m_debugDraw->SetShaded(true);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);

	Vector3f groundOffset = Vector3f::ZERO;
	groundOffset.z = -params.axles[0].offset.z -
		params.axles[0].wheelOffset.z + params.axles[0].wheels.radius;

	// Draw the body
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	m_debugDraw->DrawMesh(params.graphics.meshBody,
		vehicle->GetBody()->GetBodyToWorld(), Color::GREEN);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_debugDraw->DrawPoint(vehicle->GetBody()->GetBodyToWorld(),
		vehicle->GetBody()->GetCenterOfMass(),
		Color::MAGENTA, 6.0f);

	// Draw the wheels
	for (Wheel* wheel : vehicle->GetWheels())
	{
		float forceScale = 0.0001f;
		wheel->GetWheelToWorld();
		m_debugDraw->DrawMesh(params.graphics.meshWheel,
			wheel->GetWheelToWorld() *
			Matrix4f::CreateScale(wheel->GetRadius()), Color::WHITE);
		m_debugDraw->DrawFilledBox(
			wheel->GetWheelToWorld() *
			Matrix4f::CreateRotation(Vector3f::UNITX, -wheel->GetAngle()),
			Vector3f(0.2f, 0.3f, 0.1f), Color::MAGENTA);

		Matrix4f forceModelMatrix =
			wheel->GetWheelToWorld() *
			Matrix4f::CreateTranslation(0, 0, -1) *
			Matrix4f::CreateScale(forceScale);
		m_debugDraw->DrawLine(forceModelMatrix,
			Vector3f(0, 0, 0),
			Vector3f(0, wheel->GetLongitudinalForce(), 0),
			Color::GREEN, 4.0f);
		m_debugDraw->DrawLine(forceModelMatrix,
			Vector3f(0, 0, 0),
			Vector3f(wheel->GetLateralForce(), 0, 0),
			Color::RED, 4.0f);
		m_debugDraw->DrawLine(forceModelMatrix,
			Vector3f(0, 0, 0),
			Vector3f(wheel->GetLateralForce(), wheel->GetLongitudinalForce(), 0),
			Color::YELLOW, 4.0f);
	}

	m_debugDraw->DrawLine(Matrix4f::IDENTITY,
		pos, pos + m_forwardVector * 10, Color::GREEN);
	m_debugDraw->DrawLine(Matrix4f::IDENTITY,
		pos, pos + m_leftVector * 10, Color::RED);
	m_debugDraw->DrawLine(Matrix4f::IDENTITY,
		pos, pos + m_upVector * 10, Color::BLUE);

	glUseProgram(0);
	glLineWidth(1.0f);
	glPointSize(1.0f);
}

void DrivingApp::OnRender()
{
	Window* window = GetWindow();
	MouseState mouseState = GetMouse()->GetMouseState();
	Vector2f windowSize((float) window->GetWidth(), (float) window->GetHeight());

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDepthMask(false);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_CLAMP);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, window->GetWidth(), window->GetHeight());

	Graphics2D g(window);
	g.Clear(Color::BLACK);
	g.SetTransformation(Matrix4f::IDENTITY);


	// Draw the player
	DrawVehicle(g, m_player);


	/*
	ss << "" << endl;
	auto operatingParams = m_player->GetOperatingParams();
	ss << "Steering: " << operatingParams.steeringAngle << endl;
	ss << "Throttle: " << operatingParams.throttle << endl;
	ss << "Brake: " << operatingParams.brake << endl;
	ss << "Vehicle Speed: " << metersPerSecondToMph(m_player->GetVelocity().Length()) << " MPH" << endl;
	ss << "Position: " << m_player->GetPosition() << " MPH" << endl;
	ss << "Velocity: " << m_player->GetVelocity() << " MPH" << endl;

	g.DrawString(m_font, ss.str(), Vector2f(5, 5));
	*/

	//
	//Matrix4f projection = Matrix4f::IDENTITY;
	//Matrix4f view = m_camera.GetWorldToCameraMatrix();
	//glMatrixMode(GL_PROJECTION);
	//glLoadMatrixf(m_camera.GetViewProjectionMatrix().m);

	//// Draw HUD
	//Matrix4f projection = Matrix4f::CreateOrthographic(0.0f,
	//	(float) window->GetWidth(), (float) window->GetHeight(),
	//	0.0f, -1.0f, 1.0f);
	//glMatrixMode(GL_PROJECTION);
	//glLoadMatrixf(projection.m);

	//using namespace std;
	//std::stringstream ss;
	//ss << "GEOMETRY TEST" << endl;

	//g.DrawString(m_font, ss.str(), Vector2f(5, 5), Color::YELLOW);
	//

}
