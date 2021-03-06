#ifndef _ANIM_APP_H_
#define _ANIM_APP_H_

#include <cmgApplication/cmg_application.h>
#include <cmgMath/cmg_math.h>
#include <map>
#include <vector>
#include "Biarc.h"
#include "Biarc3.h"
#include "RoadNetwork.h"
#include "Camera.h"
#include "Driver.h"
#include "Vehicle.h"
#include "ToolSelection.h"
#include "ToolDraw.h"
#include "DrivingSystem.h"
#include "ecs/MeshRenderSystem.h"

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

	void LoadShaders();

	void OnInitialize() override;
	void OnQuit() override;
	void OnResizeWindow(int width, int height) override;
	void OnDropFiles(const Array<Path>& paths) override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

	void Reset();

	void CreateTestNetwork();

	void SetTool(EditorTool* tool);

	void UpdateCameraControls(float dt);

private:
	void DrawArc(Graphics2D& g, const Biarc3& arc, const Color& color);
	void DrawArc(Graphics2D& g, const Biarc& arc, const Color& color);
	void DrawArcs(Graphics2D& g, const BiarcPair& arcs, const Color& color);
	void DrawArc(Graphics2D& g, const Biarc& arc, float z1, float z2, float t1, float t2, const Color& color);
	void DrawArcs(Graphics2D& g, const BiarcPair& arcs, float z1, float z2, const Color& color);
	void DrawCurveLine(Graphics2D& g, const Biarc& horizontalArc, const VerticalCurve& verticalCurve, float offset, const Color& color);
	void DrawCurveLine(Graphics2D& g, const RoadCurveLine& arcs, const Color& color);

	RenderParams m_renderParams;
	RenderParams m_renderParamsHud;
	Joystick* m_joystick;
	Joystick* m_wheel;
	RoadNetwork* m_network;
	DrivingSystem* m_drivingSystem;

	// ECS
	ECS m_ecs;
	ECSSystemList m_systems;
	ECSSystemList m_renderSystems;
	MeshRenderSystem* m_meshRenderSystem;
	ArcBallControlSystem m_arcBallControlSystem;

	// Entities
	EntityHandle m_cameraEntity;
	EntityHandle m_entityPlayer;
	Camera m_camera;

	Vector2f m_mousePosition;

	DebugDraw* m_debugDraw;

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

	bool m_paused;

	Array<DebugOption*> m_debugOptions;
	DebugOption* m_showDebug;
	DebugOption* m_wireframeMode;
	DebugOption* m_showRoadMarkings;
	DebugOption* m_showEdgeLines;
	DebugOption* m_showRoadSurface;
	DebugOption* m_showNodes;
	DebugOption* m_showSeams;
	DebugOption* m_showDrivingLines;
	DebugOption* m_showCollisions;

	EditMode m_editMode;

	EditorTool* m_currentTool;
	ToolSelection* m_toolSelection;
	ToolDraw* m_toolDraw;
	Array<EditorTool*> m_tools;

	/*
	CameraState m_camera;
	CameraState m_defaultCameraState;
	Camera m_newCamera;
	Vector3f m_cameraPosition;
	Meters m_cameraDistance;
	Radians m_cameraPitch;
	Radians m_cameraYaw;*/

	Vector3f m_cursorGroundPosition;
	Vector3f m_cursorGroundPositionPrev;

	Font::sptr m_font;
	Mesh::sptr m_vehicleMesh;
	Mesh::sptr m_meshWheel;
	Shader::sptr m_shader;
	Texture::sptr m_backgroundTexture;
	Texture::sptr m_roadTexture;

	Vector2f m_backgroundPosition;
	Vector2f m_backgroundSize;
	ProfileSection m_profiling;
	ProfileSection* m_profileGeometry;
	ProfileSection* m_profileDrivers;
	ProfileSection* m_profileDraw;
	ProfileSection* m_profileNetworkSimulation;
};


void FillShape(Graphics2D& g, const Array<Vector2f>& points, const Color& color);
void FillShape(Graphics2D& g, const Array<Biarc>& arcs, const Color& color);


#endif // _ANIM_APP_H_