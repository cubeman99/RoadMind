#ifndef _EDITOR_TOOL_H_
#define _EDITOR_TOOL_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include <cmgInput/cmg_input.h>
#include <cmgGraphics/cmg_graphics.h>
#include "RoadNetwork.h"
#include "Camera.h"

struct SimulationData
{
	float time;
	float lapTime;
	float lapDistance;
	float distance;
	Vector3f position;
	float speed;
	Vector3f velocity;
	Vector3f roll;
	Vector3f pitch;
	float suspensionPositionRearLeft;
	float suspensionPositionRearRight;
	float suspensionPositionFrontLeft;
	float suspensionPositionFrontRight;
	float suspensionVelocityRearLeft;
	float suspensionVelocityRearRight;
	float suspensionVelocityFrontLeft;
	float suspensionVelocityFrontRight;
	float wheelVelocityRearLeft;
	float wheelVelocityRearRight;
	float wheelVelocityFrontLeft;
	float wheelVelocityFrontRight;
	float throttle;
	float steering;
	float brake;
	float clutch;
	float gear;
	float gForceLateral;
	float gForceLongitudinal;
	float lap;
	float engineSpeed;
	float unused1[22];
	float totalLaps;
	float trackLength;
	float unused2[1];
	float maxEngineSpeed;
	float unused3[2];
};

class EditorTool
{
	friend class MainApp;

public:
	EditorTool();

	virtual void Update(float dt) = 0;

	virtual void OnBegin() = 0;

	virtual void OnEnd() = 0;

	virtual void OnLeftMousePressed()
	{
	}

	virtual void OnRightMousePressed()
	{
	}

	virtual void OnLeftMouseReleased()
	{
	}

	virtual void OnRightMouseReleased()
	{
	}

	bool IsShiftDown() const;
	bool IsControlDown() const;
	bool IsAltDown() const;
	Vector2f GetMousePosition() const;
	Vector2f GetMousePositionInWindow() const;

public:
	RoadNetwork* m_network;
	Mouse* m_mouse;
	Keyboard* m_keyboard;
	Window* m_window;
	Vector2f m_mousePosition;
	Vector2f m_mousePositionInWindow;
	Camera* m_camera;
};


#endif // _EDITOR_TOOL_H_