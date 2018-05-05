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

	void CreateTestNetwork();
	void DrawGridFloor(const Vector3f& center, Meters squareSize, Meters gridRadius);
	void DrawConnection(Graphics2D& g, Connection* connection);
	void DrawNodeGroupConnection(Graphics2D& g, Connection* connection);
	void DrawRoadMarkings(Graphics2D& g, Connection* connection);
	void DrawNode(Graphics2D& g, Node* node);

	void SetTool(EditorTool* tool);

	//void UpdateHoverInfo();
	void UpdateCameraControls(float dt);

private:

	void DrawArcs(Graphics2D& g, const BiarcPair& arcs, const Color& color);

	SpriteFont* m_font;
	RoadNetwork* m_network;

	Vector2f m_mousePosition;

	bool m_showDebug;
	bool m_wireframeMode;

	EditMode m_editMode;

	EditorTool* m_currentTool;
	ToolSelection* m_toolSelection;
	ToolDraw* m_toolDraw;
	Array<EditorTool*> m_tools;
	Array<Driver*> m_drivers;

	CameraState m_camera;
	CameraState m_defaultCameraState;

	Texture* m_backgroundTexture;
	Vector2f m_backgroundPosition;
	Vector2f m_backgroundSize;
};


#endif // _ANIM_APP_H_