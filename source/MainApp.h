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

enum class DragState
{
	NONE,
	POSITION,
	DIRECTION,
};

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

	Node* BeginDraggingNewNode(const Vector2f& position);
	void BeginDragging(Node* node, Node* input = nullptr);
	Node* BeginExtending(Node* node, LaneSide side = LaneSide::NONE, bool reverse = false);
	void UpdateDragging();
	void CancelDragging();
	void StopDragging();

	void UpdateHoverInfo();
	void UpdateCameraControls(float dt);

private:
	RoadNetwork* m_network;

	Vector2f m_mousePosition;

	Node* m_dragNode;
	Node* m_dragInput;
	bool m_showDebug;

	DragState m_dragState;

	struct DragInfo
	{
		DragState state;
		NodeGroup* inputGroup;
		NodeGroup* nodeGroup;
		NodeGroupConnection* connection;
		Vector2f position;
	};

	DragInfo m_dragInfo;
	EditMode m_editMode;

	Array<Driver*> m_drivers;

	NodeGroup* m_lastNodeGroup;

	bool m_wireframeMode;
	int m_rightLaneCount;
	int m_leftLaneCount;

	struct
	{
		Node* node;
		LaneSide side;
		bool reverse;
		Vector2f center;
		NodeGroup* nodeGroup;
		int nodeIndex;
		float nodePartialIndex;
		int startIndex;
		bool isValidSubGroup;
	} m_hoverInfo;

	struct
	{
		Node* node;
		LaneSide side;
		bool reverse;
		Vector2f center;
	} m_snapInfo;

	CameraState m_camera;
	CameraState m_defaultCameraState;

	Texture* m_backgroundTexture;
	Vector2f m_backgroundPosition;
	Vector2f m_backgroundSize;

	Vector2f m_p1;
	Vector2f m_p2;
	Vector2f m_t1;
	Vector2f m_t2;
	float m_w;
	float m_offset;
};


#endif // _ANIM_APP_H_