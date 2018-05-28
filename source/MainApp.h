#ifndef _ANIM_APP_H_
#define _ANIM_APP_H_

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


enum class EditMode
{
	MOVE,
	CREATE,
};

class MainApp : public Application
{
public:
	MainApp();
	~MainApp();

	void UnloadShaders();
	void LoadShaders();

	void OnInitialize() override;
	void OnQuit() override;
	void OnResizeWindow(int width, int height) override;
	void OnDropFile(const String& fileName) override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

	void Reset();

	void CreateTestNetwork();
	void DrawGridFloor(const Vector3f& center, Meters squareSize, Meters gridRadius);

	void SetTool(EditorTool* tool);

	void UpdateCameraControls(float dt);

private:

	void DrawArcs(Graphics2D& g, const BiarcPair& arcs, const Color& color);
	void FillZippedArcs(Graphics2D& g, const Biarc& a, const Biarc& b, const Color& color);

	SpriteFont* m_font;
	RoadNetwork* m_network;

	Vector2f m_mousePosition;

	struct DebugOption
	{
		String name;
		bool enabled;

		DebugOption(const String& name, bool enabled)
			: name(name)
			, enabled(enabled)
		{
		}
	};

	Array<DebugOption*> m_debugOptions;
	DebugOption* m_showDebug;
	DebugOption* m_wireframeMode;
	DebugOption* m_showRoadMarkings;
	DebugOption* m_showEdgeLines;
	DebugOption* m_showRoadSurface;
	DebugOption* m_showNodes;

	EditMode m_editMode;

	EditorTool* m_currentTool;
	ToolSelection* m_toolSelection;
	ToolDraw* m_toolDraw;
	Array<EditorTool*> m_tools;
	Array<Driver*> m_drivers;

	CameraState m_camera;
	CameraState m_defaultCameraState;
	Camera m_newCamera;
	Vector3f m_cameraPosition;
	Meters m_cameraDistance;
	Radians m_cameraPitch;
	Radians m_cameraYaw;

	Vector3f m_cursorGroundPosition;
	Vector3f m_cursorGroundPositionPrev;

	Texture* m_backgroundTexture;
	Vector2f m_backgroundPosition;
	Vector2f m_backgroundSize;
};


void FillShape(Graphics2D& g, const Array<Vector2f>& points, const Color& color);
void FillShape(Graphics2D& g, const Array<Biarc>& arcs, const Color& color);

#endif // _ANIM_APP_H_