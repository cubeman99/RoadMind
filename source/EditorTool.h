#ifndef _EDITOR_TOOL_H_
#define _EDITOR_TOOL_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include <cmgInput/cmg_input.h>
#include <cmgGraphics/cmg_graphics.h>
#include "RoadNetwork.h"
#include "Camera.h"


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