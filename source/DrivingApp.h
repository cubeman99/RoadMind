#ifndef _GEOMETRY_APP_H_
#define _GEOMETRY_APP_H_

#include <cmgApplication/cmg_application.h>
#include <cmgMath/cmg_math.h>
#include <cmgPhysics/cmg_physics.h>
#include <map>
#include <vector>
#include "Biarc.h"
#include "RoadNetwork.h"
#include "Camera.h"
#include "Driver.h"
#include "ToolSelection.h"
#include "ToolDraw.h"
#include "MainApp.h"


class DrivingApp : public Application
{
public:
	DrivingApp();
	~DrivingApp();

	void OnInitialize() override;
	void OnQuit() override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

	void Reset();
	void UpdateCameraControls(float dt);
	void UpdateVehicleControls(float dt);
	void DrawVehicle(Graphics2D& g, Vehicle* vehicle);

private:
	SpriteFont* m_font;
	DebugDraw* m_debugDraw;
	PhysicsEngine* m_physicsEngine;

	Joystick* m_wheel;
	Joystick* m_joystick;

	CameraState m_camera;
	CameraState m_defaultCameraState;
	Camera m_newCamera;
	Vector3f m_cameraPosition;
	Meters m_cameraDistance;
	Radians m_cameraPitch;
	Radians m_cameraYaw;
	
	Mesh* m_vehicleMesh;
	Mesh* m_meshWheel;
	VehicleParams m_vehicleParams;
	Vehicle* m_player;
	
	Array<SimulationData> m_simulationData;
	float m_lapTime;
	Vector3f m_rollVector;
	Vector3f m_pitchVector;
	Vector3f m_upVector;
	Vector3f m_leftVector;
	Vector3f m_forwardVector;
};


#endif // _GEOMETRY_APP_H_