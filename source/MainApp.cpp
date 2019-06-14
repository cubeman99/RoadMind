#include "MainApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>
#include <iomanip>

#define BOOL2ASCII(x) (x ? "TRUE" : "FALSE")

static const char* SAVE_FILE_PATH = "road_network.rdmd";

#define ASSETS_PATH "C:/workspace/c++/cmg/RoadMind/assets/"

MainApp::MainApp()
{
	m_debugOptions.push_back(m_showRoadMarkings = new DebugOption("Markings", true));
	m_debugOptions.push_back(m_showEdgeLines = new DebugOption("Edges", true));
	m_debugOptions.push_back(m_showRoadSurface = new DebugOption("Surface", true));
	m_debugOptions.push_back(m_wireframeMode = new DebugOption("Wireframe", false));
	m_debugOptions.push_back(m_showDebug = new DebugOption("Debug", true));
	m_debugOptions.push_back(m_showNodes = new DebugOption("Nodes", true));
	m_debugOptions.push_back(m_showSeams = new DebugOption("Seams", false));
	m_debugOptions.push_back(m_showCollisions = new DebugOption("Collisions", false));
	m_debugOptions.push_back(m_showDrivingLines = new DebugOption("Paths", false));

	m_showRoadSurface->enabled = false;
	m_showCollisions->enabled = false;
	m_showDebug->enabled = true;
	m_showNodes->enabled = true;

	m_showRoadMarkings->enabled = false;
	m_showEdgeLines->enabled = true;
	m_showNodes->enabled = false;
	m_showSeams->enabled = true;
	m_wireframeMode->enabled = true;
	m_showRoadSurface->enabled = true;
}

MainApp::~MainApp()
{
	UnloadShaders();
}

void MainApp::OnInitialize()
{
	LoadShaders();

	m_paused = false;
	m_debugDraw = new DebugDraw();
	m_network = new RoadNetwork();
	m_drivingSystem = new DrivingSystem(m_network);
	m_backgroundTexture = nullptr;
	TextureParams params;
	params.SetWrap(TextureWrap::REPEAT);
	m_roadTexture = Texture::LoadTexture(ASSETS_PATH "textures/asphalt.png", params);
	
	m_defaultCameraState.m_viewHeight = 50.0f;
	m_defaultCameraState.m_position = Vector2f::ZERO;
	m_defaultCameraState.m_rotation = 0.0f;
	m_defaultCameraState.m_aspectRatio = GetWindow()->GetAspectRatio();
	m_camera = m_defaultCameraState;
	m_camera.m_aspectRatio = GetWindow()->GetAspectRatio();
	m_newCamera.SetPerspective(GetWindow()->GetAspectRatio(), 1.0f, 1.0f, 1000.0f);

	m_wheel = nullptr;
	m_joystick = nullptr;
	//m_wheel = GetInputManager()->AddDevice<Joystick>();
	//m_joystick = GetInputManager()->AddDevice<Joystick>();

	m_font = SpriteFont::LoadBuiltInFont(BuiltInFonts::FONT_CONSOLE);

	// Create the editor tools
	m_editMode = EditMode::CREATE;
	m_toolSelection = new ToolSelection();
	m_toolDraw = new ToolDraw();
	m_tools.push_back(m_toolDraw);
	m_tools.push_back(m_toolSelection);
	m_currentTool = nullptr;

	SetTool(m_toolDraw);

	Reset();
	m_network->Load(SAVE_FILE_PATH);

	//for (int i = 0; i < 50; i++)
	//	m_drivingSystem->SpawnDriver();
}

void MainApp::Reset()
{
	m_camera.m_position = m_defaultCameraState.m_position;
	m_camera.m_rotation = m_defaultCameraState.m_rotation;

	// Setup the initial camera.
	m_cameraPosition = Vector3f(0.0f, 0.0f, 0.5f);
	m_cameraPitch = Math::HALF_PI;
	m_cameraYaw = 0.0f;
	m_cameraDistance = 60.0f;
}

void MainApp::CreateTestNetwork()
{

}


//-----------------------------------------------------------------------------
// Selection
//-----------------------------------------------------------------------------

void MainApp::SetTool(EditorTool* tool)
{
	if (m_currentTool != nullptr)
		m_currentTool->OnEnd();
	m_currentTool = tool;
	if (m_currentTool != nullptr)
		m_currentTool->OnBegin();
}

void MainApp::UpdateCameraControls(float dt)
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

	// Calculate cursor ground position
	m_cursorGroundPositionPrev = m_cursorGroundPosition;
	Plane ground = Plane::XY;
	Vector2f mouseScreenCoords(
		((mousePos.x / windowSize.x) - 0.5f) * 2.0f,
		((mousePos.y / windowSize.y) - 0.5f) * -2.0f);
	Ray mouseRay = m_newCamera.GetRay(mouseScreenCoords);
	bool cursorValid = ground.CastRay(mouseRay, m_cursorGroundPosition);

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

//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

static void DrawPoint(Graphics2D& g, const Vector2f& point, const Color& color)
{
	g.FillCircle(point, 4, color);
}

static void DrawPoint(Graphics2D& g, const Vector3f& point, const Color& color)
{
	glPointSize(6.0f);
	glBegin(GL_POINTS);
	glColor4ubv(color.data());
	glVertex3fv(point.v);
	glEnd();
}

void MainApp::DrawArc(Graphics2D& g, const Biarc3& arc, const Color& color)
{
	glBegin(GL_LINE_STRIP);
	//glBegin(GL_POINTS);
	glColor4ubv(color.data());
	if (arc.IsStraight())
	{
		glVertex3fv(arc.start.v);
		glVertex3fv(arc.end.v);
	}
	else
	{
		glVertex3fv(arc.start.v);
		Vector3f v = arc.start - arc.center;
		int count = 10;
		float angle = -arc.angle / count;
		for (int j = 0; j < count; j++)
		{
			Vector3f vPrev = v;
			v.Rotate(arc.axis, angle);
			glVertex3fv((arc.center + v).v);
		}
		glVertex3fv(arc.end.v);
	}
	glEnd();
	glBegin(GL_LINE_STRIP);
	glColor4ubv(color.data());
	glVertex3fv(arc.center.v);
	glVertex3fv((arc.center + arc.axis * arc.radius).v);
	glEnd();

	glPointSize(8.0f);
	glBegin(GL_POINTS);
	glVertex3fv(arc.start.v);
	glVertex3fv(arc.end.v);
	glVertex3fv(arc.center.v);
	glEnd();
}

void MainApp::DrawArc(Graphics2D& g, const Biarc& arc, const Color& color)
{
	if (arc.radius == 0.0f)
	{
		g.DrawLine(arc.start, arc.end, color);
	}
	else
	{
		Vector2f v = arc.start;
		float angle = -arc.angle / 10.0f;
		for (int j = 0; j < 10; j++)
		{
			Vector2f vPrev = v;
			v.Rotate(arc.center, angle);
			g.DrawLine(vPrev, v, color);
		}
	}
}

void MainApp::DrawArcs(Graphics2D& g, const BiarcPair& arcs, const Color& color)
{
	DrawArc(g, arcs.first, color);
	DrawArc(g, arcs.second, color);
	if (m_showDebug->enabled)
	{
		g.FillCircle(arcs.first.start, 0.15f, color);
		g.FillCircle(arcs.first.end, 0.15f, color);
		g.FillCircle(arcs.second.end, 0.15f, color);
	}
}

static float Smooth(float t)
{
	if (t <= 0.5f)
		return (2.0f * t * t);
	else
		return (1.0f - 2.0f * (t - 1.0f) * (t - 1.0f));
}

void MainApp::DrawArc(Graphics2D& g, const Biarc& arc, float z1, float z2, float t1, float t2, const Color& color)
{
	if (arc.radius == 0.0f)
	{
		glBegin(GL_LINE_STRIP);
		glColor4ubv(color.data());
		glVertex3fv(Vector3f(arc.start, z1).v);
		glVertex3fv(Vector3f(arc.end, z2).v);
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
		glColor4ubv(color.data());
		Vector2f v = arc.start;
		int count = 10;
		float angle = -arc.angle / count;
		glVertex3fv(Vector3f(arc.start, Math::Lerp(z1, z2, Smooth(t1))).v);
		for (int j = 1; j < count; j++)
		{
			float t = Math::Lerp(t1, t2, j / (float)count);
			Vector2f vPrev = v;
			float zi = Math::Lerp(z1, z2, Smooth(t));
			v.Rotate(arc.center, angle);
			glVertex3fv(Vector3f(v, zi).v);
		}
		glVertex3fv(Vector3f(arc.end, Math::Lerp(z1, z2, Smooth(t2))).v);
		glEnd();
	}
}

void MainApp::DrawArcs(Graphics2D& g, const BiarcPair& arcs, float z1, float z2, const Color& color)
{
	float t = arcs.first.length /
		(arcs.first.length + arcs.second.length);
	//float zm = Math::Lerp(z1, z2, t);
	DrawArc(g, arcs.first, z1, z2, 0, t, color);
	DrawArc(g, arcs.second, z1, z2, t, 1, color);
	//BiarcPair zarc = BiarcPair::Interpolate(Vector2f(0, z1), Vector2f::UNITX,
	//Vector2f(arcs.Length(), z2), Vector2f::UNITX);
	if (m_showDebug->enabled)
	{
		g.FillCircle(arcs.first.start, 0.15f, color);
		g.FillCircle(arcs.first.end, 0.15f, color);
		g.FillCircle(arcs.second.end, 0.15f, color);
	}
}

void MainApp::DrawCurveLine(Graphics2D& g, const Biarc& horizontalArc,
	const VerticalCurve& verticalCurve, float offset, const Color& color)
{
	glBegin(GL_LINE_STRIP);
	glColor4ubv(color.data());
	glVertex3fv(Vector3f(horizontalArc.start,
		verticalCurve.GetHeightFromDistance(offset)).v);

	if (!horizontalArc.IsStraight())
	{
		Vector3f point(horizontalArc.start, 0.0f);
		int count = 10;
		float angle = -horizontalArc.angle / count;
		for (int j = 1; j < count; j++)
		{
			float t = j / (float) count;
			point.xy.Rotate(horizontalArc.center, angle);
			point.z = verticalCurve.GetHeightFromDistance(offset + (horizontalArc.length * t));
			glVertex3fv(point.v);
		}
	}

	glVertex3fv(Vector3f(horizontalArc.end,
		verticalCurve.GetHeightFromDistance(offset + horizontalArc.length)).v);
	glEnd();
}

void MainApp::DrawCurveLine(Graphics2D& g,
	const RoadCurveLine& curve, const Color& color)
{
	float tm = Math::Lerp(curve.t1, curve.t2,
		curve.horizontalCurve.first.length /
		(curve.horizontalCurve.first.length +
			curve.horizontalCurve.second.length));

	if (m_showDebug->enabled)
	{
		DrawPoint(g, curve.Start(), color);
		DrawPoint(g, curve.Middle(), color);
		DrawPoint(g, curve.End(), color);
	}

	DrawCurveLine(g, curve.horizontalCurve.first,
		curve.verticalCurve, 0.0f, color);
	DrawCurveLine(g, curve.horizontalCurve.second,
		curve.verticalCurve, curve.horizontalCurve.first.length, color);
}

void MainApp::FillZippedArcs(Graphics2D& g,
	const Biarc& a, const Biarc& b, const Color& color)
{
	float maxLength = Math::Max(a.length, b.length);
	int count = 10;
	float interval = 1.5f;
	count = (int)((maxLength / interval) + 0.5f);
	count = Math::Max(2, count);
	glBegin(GL_TRIANGLE_STRIP);
	glColor3ubv(color.data());
	glVertex2fv(a.start.v);
	glVertex2fv(b.start.v);
	for (int i = 1; i <= count; i++)
	{
		float t = (float)i / count;
		glVertex2fv(a.GetPoint(a.length * t).v);
		glVertex2fv(b.GetPoint(b.length * t).v);
	}
	glVertex2fv(a.end.v);
	glVertex2fv(b.end.v);
	glEnd();
}

void MainApp::FillZippedCurves(Graphics2D& g, const RoadCurveLine& a,
	const RoadCurveLine& b, const Color& color)
{
	float interval = 1.5f;

	glBegin(GL_TRIANGLE_STRIP);
	glColor3ubv(color.data());
	//glVertex3fv(a.Start().v);
	//glVertex3fv(b.Start().v);

	for (int k = 0; k < 2; k++)
	{
		float maxLength = Math::Max(a.horizontalCurve.arcs[k].length,
			b.horizontalCurve.arcs[k].length);
		int count = Math::Max(2, (int)((maxLength / interval) + 0.5f));
		float offset1 = a.horizontalCurve.first.length * k;
		float offset2 = b.horizontalCurve.first.length * k;
		for (int i = 0; i < count; i++)
		{
			float t = (float)i / (float)count;
			glVertex3fv(a.GetPoint(offset1 + a.horizontalCurve.arcs[k].length * t).v);
			glVertex3fv(b.GetPoint(offset2 + b.horizontalCurve.arcs[k].length * t).v);
		}
	}

	glVertex3fv(a.End().v);
	glVertex3fv(b.End().v);
	glEnd();

}

void MainApp::DrawGridFloor(const Vector3f& center, Meters squareSize, Meters gridRadius)
{
	int majorTick = 10;
	int majorMajorTick = 100;
	Color colors[3];
	colors[0] = Color(10, 10, 10);
	colors[1] = Color(50, 50, 50);
	colors[2] = Color(120, 120, 120);

	// Snap the grid radius to the square size.
	gridRadius = Math::Ceil(gridRadius / squareSize) * squareSize;

	float startX = center.x - gridRadius;
	float startZ = center.y - gridRadius;
	int indexX = (int)Math::Floor(startX / (squareSize));
	int indexZ = (int)Math::Floor(startZ / (squareSize));
	startX = indexX * squareSize;
	startZ = indexZ * squareSize;
	float endX = startX + (gridRadius * 2.0f);
	float endZ = startZ + (gridRadius * 2.0f);
	float x = startX;
	float z = startZ;

	// Draw a grid of perpendicular lines.
	glDepthMask(false);
	glBegin(GL_LINES);
	for (; x < center.x + gridRadius; x += squareSize, z += squareSize, indexX++, indexZ++)
	{
		// Draw line along z-axis.
		int major = 0;
		if (indexX % majorTick == 0)
			major = 1;
		if (indexX % majorMajorTick == 0)
			major = 2;
		glColor4ubv(colors[major].data());
		//glVertex3f(x, center.y, startZ);
		//glVertex3f(x, center.y, endZ);
		glVertex3f(x, startZ, center.z);
		glVertex3f(x, endZ, center.z);

		// Draw line along x-axis.
		major = 0;
		if (indexZ % majorTick == 0)
			major = 1;
		if (indexZ % majorMajorTick == 0)
			major = 2;
		glColor4ubv(colors[major].data());
		//glVertex3f(startX, center.y, z);
		//glVertex3f(endX, center.y, z);
		glVertex3f(startX, z, center.z);
		glVertex3f(endX, z, center.z);
	}
	glEnd();
	glDepthMask(true);
}



//-----------------------------------------------------------------------------
// Overridden Methods
//-----------------------------------------------------------------------------

void MainApp::LoadShaders()
{
}

void MainApp::UnloadShaders()
{
}

void MainApp::OnQuit()
{
	delete m_debugDraw;
	m_debugDraw = nullptr;
	delete m_vehicleMesh;
	m_vehicleMesh = nullptr;
	delete m_meshWheel;
	m_meshWheel = nullptr;
	delete m_drivingSystem;
	delete m_roadTexture;
	m_roadTexture = nullptr;

	m_drivingSystem = nullptr;

	delete m_network;
	m_network = nullptr;

	delete m_font;
	m_font = nullptr;

	if (m_backgroundTexture != nullptr)
	{
		delete m_backgroundTexture;
		m_backgroundTexture = nullptr;
	}
}

void MainApp::OnResizeWindow(int width, int height)
{
	m_camera.m_aspectRatio = GetWindow()->GetAspectRatio();
	m_newCamera.SetAspectRatio(GetWindow()->GetAspectRatio());
}

void MainApp::OnDropFile(const String& fileName)
{
	if (m_backgroundTexture != nullptr)
	{
		delete m_backgroundTexture;
		m_backgroundTexture = nullptr;
	}

	Texture* texture = Texture::LoadTexture(fileName);

	if (texture != nullptr)
	{
		m_backgroundTexture = texture;

		Rect2f viewRect = m_camera.GetRect();
		m_backgroundSize = viewRect.size;
		m_backgroundPosition = viewRect.GetCenter();
		printf("Loaded background image: %s\n", fileName.c_str());
	}
	else
	{
		printf("Error background image: %s\n", fileName.c_str());
	}
}

void MainApp::OnUpdate(float dt)
{
	Mouse* mouse = GetMouse();
	Keyboard* keyboard = GetKeyboard();
	Window* window = GetWindow();
	MouseState mouseState = mouse->GetMouseState();
	Vector2f windowSize((float)window->GetWidth(), (float)window->GetHeight());
	m_mousePosition.x = (float)mouseState.x;
	m_mousePosition.y = (float)mouseState.y;
	m_mousePosition = m_camera.ToWorldSpace(m_mousePosition, windowSize);

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

	// F4: Toggle Fullscreen Mode
	if (keyboard->IsKeyPressed(Keys::f4))
		GetWindow()->SetFullscreen(!GetWindow()->IsFullscreen());

	// Ctrl+S: Save
	if (ctrl && keyboard->IsKeyPressed(Keys::s))
	{
		m_network->Save(SAVE_FILE_PATH);
		std::cout << "Saved to " << SAVE_FILE_PATH << std::endl;
	}
	// Ctrl+S: Load
	if (ctrl && keyboard->IsKeyPressed(Keys::l))
	{
		m_network->Load(SAVE_FILE_PATH);
		std::cout << "Loaded " << SAVE_FILE_PATH << std::endl;
	}

	// Home: reset camera
	if (keyboard->IsKeyPressed(Keys::home))
	{
		m_camera.m_position = m_defaultCameraState.m_position;
		m_camera.m_rotation = m_defaultCameraState.m_rotation;
	}

	// Number keys: debug options
	for (unsigned int i = 0; i < m_debugOptions.size(); i++)
	{
		if (!ctrl && keyboard->IsKeyPressed(Keys::n1 + i))
			m_debugOptions[i]->enabled = !m_debugOptions[i]->enabled;
	}

	//// F1: Toggle debug display
	//if (keyboard->IsKeyPressed(Keys::f1))
	//	m_showDebug = !m_showDebug;

	// Ctrl+R: Clear road network
	if (ctrl && keyboard->IsKeyPressed(Keys::r))
	{
		m_toolDraw->CancelDragging();
		m_drivingSystem->Clear();
		m_network->ClearNodes();
		Reset();
	}
	if (keyboard->IsKeyPressed(Keys::backspace))
		m_drivingSystem->Clear();

	// T: Create test network
	if (!ctrl && keyboard->IsKeyPressed(Keys::t))
	{
		m_toolDraw->CancelDragging();
		m_network->ClearNodes();
		CreateTestNetwork();
	}

	if (keyboard->IsKeyPressed(Keys::f5))
		m_paused = !m_paused;
	if (keyboard->IsKeyPressed(Keys::f6))
		m_paused = false;

	// M: Selection tool
	if (!ctrl && keyboard->IsKeyPressed(Keys::m))
		SetTool(m_toolSelection);
	// P: Draw tool
	if (!ctrl && keyboard->IsKeyPressed(Keys::p))
		SetTool(m_toolDraw);
	if (keyboard->IsKeyPressed(Keys::tab))
	{
		if (m_currentTool == m_toolDraw)
			SetTool(m_toolSelection);
		else
			SetTool(m_toolDraw);
	}

	// Enter: Spawn driver
	if (keyboard->IsKeyPressed(Keys::enter))
	{
		int count = 1;
		if (ctrl)
			count = 10;
		for (int i = 0;  i < count; i++)
			m_drivingSystem->SpawnDriver();
	}

	// Update the current tool
	if (m_currentTool != nullptr)
	{
		m_currentTool->m_keyboard = GetKeyboard();
		m_currentTool->m_mouse = GetMouse();
		m_currentTool->m_network = m_network;
		m_currentTool->m_mousePosition = m_cursorGroundPosition.xy;
		m_currentTool->m_mousePositionInWindow = Vector2f(
			(float)mouseState.x, (float)mouseState.y);
		m_currentTool->m_camera = &m_newCamera;
		m_currentTool->m_window = GetWindow();

		if (mouse->IsButtonPressed(MouseButtons::left))
			m_currentTool->OnLeftMousePressed();
		if (!ctrl && mouse->IsButtonPressed(MouseButtons::right))
			m_currentTool->OnRightMousePressed();
		if (mouse->IsButtonReleased(MouseButtons::left))
			m_currentTool->OnLeftMouseReleased();
		if (mouse->IsButtonReleased(MouseButtons::right))
			m_currentTool->OnRightMouseReleased();

		m_currentTool->Update(dt);
	}
	

	UpdateCameraControls(dt);

	m_network->UpdateNodeGeometry();
	if (!m_paused)
	{
		m_network->Simulate(dt);
		m_drivingSystem->Update(dt);
	}

	if (keyboard->IsKeyPressed(Keys::f6))
		m_paused = true;
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


void FillShape(Graphics2D& g, const Array<Vector2f>& points, const Color& color)
{
	Array<Vector2f> vertices;
	Convexity convexity;
	Vector2f a, b, c;
	Array<Vector2f> remaining = points;
	unsigned int count = remaining.size();
	unsigned int searched = 0;

	for (int i = 0; count >= 3 && searched < count; i = (i + 1) % count)
	{
		a = remaining[i];
		b = remaining[(i + 1) % count];
		c = remaining[(i + 2) % count];

		convexity = GetConvexity(a, b, c);

		if (a.DistToSqr(b) < 0.0001f)
		{
			remaining.erase(remaining.begin() + ((i + 1) % count));
			if (i == count - 1)
				i--;
			count--;
			searched = 0;
			i--;
		}
		else if (convexity == Convexity::STRAIGHT)
		{
			searched++;
		}
		else
		{
			bool contains = false;
			for (unsigned int j = 0; j < points.size(); j++)
			{
				Vector2f v = points[j];
				if (IsInsideTriangle(a, b, c, v))
				{
					contains = true;
					break;
				}
			}

			if (!contains && convexity == Convexity::CONVEX)
			{
				vertices.push_back(a);
				vertices.push_back(b);
				vertices.push_back(c);
				if (i == count - 1)
				{
					i--;
					remaining.erase(remaining.begin());
				}
				else
				{
					remaining.erase(remaining.begin() + ((i + 1) % count));
				}
				i--;
				count--;
				searched = 0;
			}
			else
			{
				searched++;
			}
		}
	}

	// Draw the shape
	glBegin(GL_TRIANGLES);
	glColor4ubv(color.data());
	for (unsigned int i = 0; i < vertices.size(); i++)
		glVertex2fv(vertices[i].v);
	glEnd();
}

void FillShape2(Graphics2D& g, const Array<Vector3f>& left, const Array<Vector3f>& right, const Color& color)
{
	const Array<Vector3f>* sides[2] = { &left, &right };
	Array<Vector3f> vertices;
	unsigned int head[2] = { 1, 0 };
	Vector3f a, b, c;
	int side = 0;
	bool prevConvex = true;
	Vector3f mins = left[0];
	for (unsigned int axis = 0; axis < 2; axis++)
	{
		for (unsigned int i = 0; i < left.size(); i++)
			mins[axis] = Math::Min(mins[axis], left[i][axis]);
		for (unsigned int i = 0; i < right.size(); i++)
			mins[axis] = Math::Min(mins[axis], right[i][axis]);
	}

	while (head[0] < left.size() && head[1] < right.size())
	{
		int other = 1 - side;

		// Remove equivilant vertices
		a = sides[side]->at(head[side] - 1);
		b = sides[side]->at(head[side]);
		if (a.xy.DistToSqr(b.xy) < 0.001f)
		{
			head[side] += 1;
			continue;
		}
		a = sides[side]->at(head[side] - 1);
		b = sides[other]->at(head[other]);
		if (a.xy.DistToSqr(b.xy) < 0.001f)
		{
			head[side] += 1;
			continue;
		}

		// Define the triangle in clockwise order
		if (side == 0)
		{
			a = sides[other]->at(head[other]);
			b = sides[side]->at(head[side] - 1);
			c = sides[side]->at(head[side]);
		}
		else
		{
			a = sides[side]->at(head[side] - 1);
			b = sides[other]->at(head[other]);
			c = sides[side]->at(head[side]);
		}

		// If this triangle will be concave, then switch to the other side
		Convexity convexity = GetConvexity(a.xy, b.xy, c.xy);
		if (prevConvex && convexity == Convexity::CONCAVE)
		{
			head[side] -= 1;
			head[other] += 1;
			prevConvex = false;
			side = other;
			continue;
		}
		prevConvex = true;

		vertices.push_back(a);
		vertices.push_back(b);
		vertices.push_back(c);

		// Switch to the other side
		if (head[other] < sides[other]->size() - 1)
			side = other;
		head[side] += 1;
	}

	// Draw the shape
	glBegin(GL_TRIANGLES);
	glColor4ubv(color.data());
	Vector2f texOffset = Vector2f::ZERO;
	for (unsigned int axis = 0; axis < 2; axis++)
	{
		if (mins[axis] < 0)
			texOffset[axis] += (int) (mins[axis] - 5);
	}
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		Vector2f texCoord = (vertices[i].xy / 5.0f) + texOffset;
		glTexCoord2fv(texCoord.v);
		glVertex3fv(vertices[i].v);
	}
	glEnd();
}

void FillShape(Graphics2D& g, const Array<Biarc>& arcs, const Color& color)
{
	Array<Vector2f> vertices;
	for (unsigned int i = 0; i < arcs.size(); i++)
	{
		Biarc arc = arcs[i];
		if (!arc.IsPoint())
			vertices.push_back(arc.start);
		if (!arc.IsStraight() && !arc.IsPoint())
		{
			Vector2f v = arc.start;
			int count = (int)((Math::Abs(arc.angle) / Math::TWO_PI) * 50) + 2;
			//float angle = arc.angle / count;
			//v.Rotate(arc.center, angle);
			for (int j = 1; j < count; j++)
			{
				float t = j / (float)count;
				float angle = -arc.angle * t;
				v = arc.start;
				v.Rotate(arc.center, angle);
				//Vector2f vPrev = v;
				//v.Rotate(arc.center, angle);
				vertices.push_back(v);
			}
		}
		vertices.push_back(arc.end);
	}
	FillShape(g, vertices, color);
}


void FillShape2(Graphics2D& g, const Array<RoadCurveLine>& left, const Array<RoadCurveLine>& right, const Color& color)
{
	Array<Vector3f> vertices[2];
	const Array<RoadCurveLine>* sides[2] = { &left, &right };

	for (int side = 0; side < 2; side++)
	{
		for (unsigned int i = 0; i < sides[side]->size(); i++)
		{
			const RoadCurveLine& curve = sides[side]->at(i);
			float dist = 0.0f;
			vertices[side].push_back(curve.Start());
			for (unsigned int half = 0; half < 2; half++)
			{
				Biarc arc = curve.horizontalCurve.arcs[half];
				if (arc.IsPoint())
					continue;
				if (!arc.IsStraight() && !arc.IsPoint())
				{
					Vector2f v = arc.start;
					int count = (int)((Math::Abs(arc.angle) / Math::TWO_PI) * 50) + 2;
					//float angle = arc.angle / count;
					for (int j = 1; j < count; j++)
					{
						float t = j / (float) count;
						float angle = -arc.angle * t;
						v = arc.start;
						v.Rotate(arc.center, angle);
						float distAlongCurve = dist + (arc.length * t);
						//float curveT = distAlongCurve / curve.Length();
						//float z = curve.verticalCurve.GetHeight(curveT);
						float z = curve.verticalCurve.GetHeightFromDistance(distAlongCurve);
						vertices[side].push_back(Vector3f(v, z));
					}
				}
				if (half == 0)
					vertices[side].push_back(curve.Middle());
				dist += arc.length;
			}
			vertices[side].push_back(curve.End());
			continue;/*

			if (vertices[side].size() == 0)
				vertices[side].push_back(arc.start);
			if (arc.IsPoint() && (vertices[side].size() > 0 || i + 1 < sides[side]->size()))
				continue;
			if (!arc.IsStraight() && !arc.IsPoint())
			{
				Vector2f v = arc.start;
				int count = (int)((Math::Abs(arc.angle) / Math::TWO_PI) * 50) + 2;
				//float angle = arc.angle / count;
				//v.Rotate(arc.center, angle);
				for (int j = 1; j < count; j++)
				{
					float t = j / (float)count;
					float angle = -arc.angle * t;
					v = arc.start;
					v.Rotate(arc.center, angle);
					//Vector2f vPrev = v;
					//v.Rotate(arc.center, angle);
					vertices[side].push_back(v);
				}
			}
			vertices[side].push_back(arc.end);*/
		}
	}
	FillShape2(g, vertices[0], vertices[1], color);
}


void MainApp::OnRender()
{
	Window* window = GetWindow();
	MouseState mouseState = GetMouse()->GetMouseState();
	Vector2f windowSize((float)window->GetWidth(),
		(float)window->GetHeight());

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

	//m_camera.m_aspectRatio = GetWindow()->GetAspectRatio();
	//Matrix4f projection = Matrix4f::CreateOrthographic(0.0f, window->GetWidth(), window->GetHeight(), 0.0f, -1.0f, 1.0f);
	Matrix4f projection = Matrix4f::IDENTITY;
	Matrix4f view = m_camera.GetWorldToCameraMatrix();
	//view = Matrix4f::IDENTITY;
	glMatrixMode(GL_PROJECTION);
	//glLoadMatrixf((projection * view).m);
	glLoadMatrixf(m_newCamera.GetViewProjectionMatrix().m);

	if (m_backgroundTexture != nullptr)
	{
		g.DrawTexture(m_backgroundTexture,
			m_backgroundPosition - (m_backgroundSize * 0.5f));
	}

	Color colorEdgeLines = Color::GRAY;

	float r = 0.2f;
	float r2 = 0.4f;
	Color c = Color::WHITE;

	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	Vector3f gridCenter;
	gridCenter.z = 0.0f;
	gridCenter.xy = m_cameraPosition.xy;
	Meters gridRadius = m_camera.m_viewHeight;
	gridRadius = m_cameraDistance * 2.0f;
	DrawGridFloor(gridCenter, 1.0f, gridRadius);

	if (m_wireframeMode->enabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	RoadMetrics metrics = m_network->GetMetrics();

	// Draw road surfaces
	RandomNumberGenerator rng;
	rng.SetSeed(1);
	if (m_showRoadSurface->enabled)
	{
		// Draw lane surfaces
		Color colorRoadFill = Color(30, 30, 30);
		int index = 0;
		for (NodeGroupConnection* connection : m_network->GetNodeGroupConnections())
		{
			NodeGroupConnection* twin = connection->GetTwin();
			RoadCurveLine leftEdge;
			RoadCurveLine rightEdge = connection->GetRightVisualShoulderLine();

			if (twin != nullptr)
			{
				if (connection->GetId() < twin->GetId())
					leftEdge = twin->GetRightVisualShoulderLine().Reverse();
				else
					continue;
			}
			else
			{
				leftEdge = connection->GetLeftVisualShoulderLine();
			}

			//FillZippedCurves(g, leftEdge, rightEdge, colorRoadFill);

			
			Color c(Vector3f(rng.NextFloat(0.5f, 1.0f),
			rng.NextFloat(0.5f, 1.0f),
			rng.NextFloat(0.5f, 1.0f)));

			auto seamsIL = connection->GetSeams(IOType::INPUT, LaneSide::LEFT);
			auto seamsIR = connection->GetSeams(IOType::INPUT, LaneSide::RIGHT);
			auto seamsOL = connection->GetSeams(IOType::OUTPUT, LaneSide::LEFT);
			auto seamsOR = connection->GetSeams(IOType::OUTPUT, LaneSide::RIGHT);

			Array<RoadCurveLine> leftContour;
			Array<RoadCurveLine> rightContour;

			// Left
			for (auto it = seamsIL.begin(); it != seamsIL.end(); it++)
				leftContour.push_back(*it);
			leftContour.push_back(leftEdge);
			for (auto it = seamsOL.begin(); it != seamsOL.end(); it++)
				leftContour.push_back(*it);

			// Right
			for (auto it = seamsIR.begin(); it != seamsIR.end(); it++)
				rightContour.push_back(*it);
			rightContour.push_back(rightEdge);
			for (auto it = seamsOR.begin(); it != seamsOR.end(); it++)
				rightContour.push_back(*it);

			glBindTexture(GL_TEXTURE_2D, m_roadTexture->GetGLTextureID());
			FillShape2(g, leftContour, rightContour, Color::WHITE);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// Draw intersection surfaces
		for (RoadIntersection* intersection : m_network->GetIntersections())
		{
			auto edges = intersection->GetEdges();
			Array<Biarc> contour;
			for (unsigned int i = 0; i < edges.size(); i++)
			{
				RoadIntersectionEdge* prevEdge = edges[i];
				RoadIntersectionEdge* edge = edges[(i + 1) % edges.size()];

				contour.push_back(Biarc::CreateLine(
					prevEdge->GetShoulderEdge().first.start,
					edge->GetShoulderEdge().second.end));
				contour.push_back(edge->GetShoulderEdge().Reverse().first);
				contour.push_back(edge->GetShoulderEdge().Reverse().second);
			}
			FillShape(g, contour, colorRoadFill);
		}
	}

	// Draw road markings
	Matrix4f tt = Matrix4f::CreateTranslation(0.0f, 0.0f, 0.1f);
	g.SetTransformation(tt);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(tt.data());
	for (NodeGroupConnection* connection : m_network->GetNodeGroupConnections())
	{
		NodeGroupConnection* twin = connection->GetTwin();

		// Draw shoudler edges
		if (m_showEdgeLines->enabled)
		{
			if (twin == nullptr)
				DrawCurveLine(g, connection->m_visualShoulderLines[0], colorEdgeLines);
			DrawCurveLine(g, connection->m_visualShoulderLines[1], colorEdgeLines);

			//DrawArcs(g, connection->m_visualShoulderLines[0],
			//	connection->GetInput().group->GetPosition().z,
			//	connection->GetOutput().group->GetPosition().z,
			//	colorEdgeLines);
			//DrawArcs(g, connection->m_visualShoulderLines[1],
			//	connection->GetInput().group->GetPosition().z,
			//	connection->GetOutput().group->GetPosition().z,
			//	colorEdgeLines);
		}

		// Draw seams
		if (m_showSeams->enabled)
		{
			for (RoadCurveLine seam : connection->GetSeams(IOType::INPUT, LaneSide::LEFT))
				DrawCurveLine(g, seam, Color::MAGENTA);
			for (RoadCurveLine seam : connection->GetSeams(IOType::INPUT, LaneSide::RIGHT))
				DrawCurveLine(g, seam, Color::MAGENTA);
			for (RoadCurveLine seam : connection->GetSeams(IOType::OUTPUT, LaneSide::LEFT))
				DrawCurveLine(g, seam, Color::MAGENTA);
			for (RoadCurveLine seam : connection->GetSeams(IOType::OUTPUT, LaneSide::RIGHT))
				DrawCurveLine(g, seam, Color::MAGENTA);
		}

		// Draw lane divider lines
		if (m_showRoadMarkings->enabled)
		{
			//DrawArc(g, connection->m_arc1, Color::CYAN);
			//DrawArc(g, connection->m_arc2, Color::CYAN);
			//curve = RoadCurveLine(connection->m_visualEdgeLines[0],
			//	connection->GetInput().group->GetPosition().z,
			//	connection->GetOutput().group->GetPosition().z);
			//DrawCurveLine(g, curve, Color::YELLOW);
			//for (unsigned int i = 1; i < connection->m_dividerLines.size() - 1; i++)
			//{
			//	curve.horizontalCurve = connection->m_dividerLines[i];
			//	DrawCurveLine(g, curve, Color::YELLOW);
			//}
			//curve.horizontalCurve = connection->m_visualEdgeLines[1];
			//DrawCurveLine(g, curve, Color::WHITE);

			//DrawArcs(g, connection->m_visualEdgeLines[0],
			//	connection->GetInput().group->GetPosition().z,
			//	connection->GetOutput().group->GetPosition().z,
			//	Color::YELLOW);
			//for (unsigned int i = 1; i < connection->m_dividerLines.size() - 1; i++)
			//{
			//	DrawArcs(g, connection->m_dividerLines[i],
			//		connection->GetInput().group->GetPosition().z,
			//		connection->GetOutput().group->GetPosition().z,
			//		Color::WHITE);
			//}
			//DrawArcs(g, connection->m_visualEdgeLines[1],
			//	connection->GetInput().group->GetPosition().z,
			//	connection->GetOutput().group->GetPosition().z,
			//	Color::WHITE);

			//if (twin != nullptr)
			//DrawCurveLine(g, connection->m_visualEdgeLines[0], Color::GREEN);
			//else
			DrawCurveLine(g, connection->m_visualEdgeLines[0], Color::YELLOW);
			for (unsigned int i = 1; i < connection->m_dividerLines.size() - 1; i++)
				DrawArcs(g, connection->m_dividerLines[i], Color::WHITE);
			DrawCurveLine(g, connection->m_visualEdgeLines[1], Color::WHITE);
		}
	}

	for (RoadIntersection* intersection : m_network->GetIntersections())
	{
		for (RoadIntersectionEdge* edge : intersection->GetEdges())
		{
			BiarcPair shoulderEdge = edge->GetShoulderEdge();
			BiarcPair laneEdge = edge->GetLaneEdge();


			// Draw shoulder edge
			if (m_showEdgeLines->enabled)
				DrawArcs(g, shoulderEdge, colorEdgeLines);

			// Draw lane edge
			if (m_showRoadMarkings->enabled)
			{
				IOType leftType = edge->GetPoint(LaneSide::LEFT)->GetIOType();
				IOType rightType = edge->GetPoint(LaneSide::RIGHT)->GetIOType();
				if (leftType != rightType)
				{
					if (rightType == IOType::INPUT)
						c = Color::YELLOW;
					else
						c = Color::WHITE;
					DrawArcs(g, laneEdge, c);
				}
			}
		}
	}
	g.SetTransformation(Matrix4f::IDENTITY);

	// Draw node groups
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		Vector2f center = group->GetPosition().xy;

		for (int i = 0; i < group->GetNumNodes(); i++)
		{
			Node* node = group->GetNode(i);

			// Draw stop line at end of lane
			if (m_showRoadMarkings->enabled && group->GetOutputs().empty())
				g.DrawLine(node->GetLeftEdge().xy, node->GetRightEdge().xy, Color::WHITE);

			if (m_showNodes->enabled)
			{
				Vector3f center = node->GetCenter();
				g.DrawLine(node->GetLeftEdge().xy, node->GetRightEdge().xy, Color::WHITE);
				g.FillCircle(node->GetRightEdge().xy, r, Color::WHITE);
				g.DrawCircle(center.xy, node->GetWidth() * 0.5f, Color::WHITE);
				g.DrawLine(center.xy, center.xy +
					(node->GetDirection() * node->GetWidth() * 0.5f),
					Color::WHITE);
				Color signalColor = Color::BLACK;
				TrafficLightSignal signal = node->GetSignal();
				if (signal == TrafficLightSignal::GO)
					signalColor = Color::GREEN;
				else if (signal == TrafficLightSignal::STOP)
					signalColor = Color::RED;
				else if (signal == TrafficLightSignal::GO_YIELD)
					signalColor = Color::GREEN;
				else if (signal == TrafficLightSignal::YELLOW)
					signalColor = Color::YELLOW;
				else if (signal == TrafficLightSignal::STOP_SIGN)
					signalColor = Color::DARK_RED;
				g.DrawCircle(center.xy, node->GetWidth() * 0.4f, signalColor);
			}
		}
	}

	// Draw node ties
	for (NodeGroupTie* tie : m_network->GetNodeGroupTies())
	{
		if (m_showDebug->enabled)
		{
			g.FillCircle(tie->GetPosition().xy, r * 2.0f, Color::RED);
		}
	}

	// Draw drivers
	for (Driver* driver : m_drivingSystem->GetDrivers())
	{
		Meters radius = 0.75f;
		float slowPercent = driver->GetSlowDownPercent();
		Color driverColor = Color::Lerp(Color::GREEN, Color::RED, slowPercent);
		Color outlineColor = Color::WHITE;
		Color brakeColorLeft = Color(80, 0, 0);
		Color brakeColorRight = Color(80, 0, 0);
		Color lightColorLeft = Color(80, 80, 0);
		Color lightColorRight = Color(80, 80, 0);
		auto lights = driver->GetLightState();
		if (lights.braking)
		{
			brakeColorLeft = Color::RED;
			brakeColorRight = Color::RED;
		}
		if (lights.leftBlinker)
		{
			lightColorLeft = Color::YELLOW;
			brakeColorLeft = Color::YELLOW;
		}
		if (lights.rightBlinker)
		{
			lightColorRight = Color::YELLOW;
			brakeColorRight = Color::YELLOW;
		}
		outlineColor = Color::Lerp(Color::GREEN, Color::RED, slowPercent);
		if (driver->IsColliding())
			outlineColor = Color::DARK_RED;
		driverColor = Color::BLACK;
		auto params = driver->GetVehicleParams();
		Meters lightLength = 0.2f;

		if (m_showCollisions->enabled)
		{
			for (int i = 0; i < DRIVER_MAX_FUTURE_STATES; i++)
			{
				auto state = driver->GetState(i);
				if (driver->m_collisionIndex == i)
				{
					for (int j = 0; j < params.trailerCount; j++)
					{
						Vector3f size = params.size[j];
						Vector2f dir = state.direction[j];
						dir.Normalize();
						Matrix3f dcm = Matrix3f(
							Vector3f(dir.x, dir.y, 0.0f),
							Vector3f(dir.y, -dir.x, 0.0f),
							Vector3f::UNITZ);
						g.SetTransformation(Matrix4f::CreateTranslation(0.0f, 0.0f, 0.2f) *
							Matrix4f::CreateTranslation(state.position[j]) * Matrix4f(dcm));
						Color futureColor = Color::GRAY;
						futureColor = driver->m_futureCollision ? Color::YELLOW : Color::MAGENTA;
						g.DrawRect(-size.x * 0.5f, -size.y * 0.5f, size.x, size.y, futureColor);
					}
				}
			}
		}
		
		for (int i = 0; i < params.trailerCount; i++)
		{
			auto state = driver->GetState();
			Vector3f size = params.size[i];
			Vector2f dir = state.direction[i];
			dir.Normalize();
			Matrix3f dcm = Matrix3f(
				Vector3f(dir.x, dir.y, 0.0f),
				Vector3f(dir.y, -dir.x, 0.0f),
				Vector3f::UNITZ);
			g.SetTransformation(Matrix4f::CreateTranslation(0.0f, 0.0f, 0.2f) *
				Matrix4f::CreateTranslation(state.position[i]) * Matrix4f(dcm));
			
			g.FillRect(-size.x * 0.5f, -size.y * 0.5f, size.x, size.y, driverColor);
			lightLength = size.y * 0.2f;
			if (i == 0)
			{
				g.FillRect(size.x * 0.5f - lightLength, -size.y * 0.5f, lightLength, size.y * 0.25f, lightColorLeft);
				g.FillRect(size.x * 0.5f - lightLength, size.y * 0.25f, lightLength, size.y * 0.25f, lightColorRight);
			}
			g.FillRect(-size.x * 0.5f, -size.y * 0.5f, lightLength, size.y * 0.25f, brakeColorLeft);
			g.FillRect(-size.x * 0.5f, size.y * 0.25f, lightLength, size.y * 0.25f, brakeColorRight);
			if (i < params.trailerCount - 1)
			{
				/*g.DrawLine(Vector2f(-size.x * 0.5f, 0.0f),
					Vector2f(-size.x * 0.5f - params.pivotOffset[0], 0.0f),
					outlineColor);*/
			}
			if (i > 0)
			{
				/*g.DrawLine(Vector2f(size.x * 0.5f, 0.0f),
					Vector2f(size.x * 0.5f + params.pivotOffset[0], 0.0f),
					outlineColor);*/
				g.DrawCircle(Vector2f(size.x * 0.5f + params.pivotOffset[0], 0.0f),
					size.y * 0.1f, outlineColor);
			}
			g.DrawRect(-size.x * 0.5f, -size.y * 0.5f, size.x, size.y, outlineColor);
		}

		g.SetTransformation(
			Matrix4f::CreateTranslation(driver->GetPosition()) *
			Matrix4f::CreateScale(0.1f, -0.1f, 1.0f));
		std::stringstream ss;
		//ss << driver->GetId();
		//ss << (int) driver->GetMovementState();
		//ss << driver->GetAcceleration();
		g.DrawString(m_font, ss.str(), Vector2f::ZERO, Color::WHITE, TextAlign::CENTERED);
	}
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	g.SetTransformation(Matrix4f::IDENTITY);

	// Draw driver paths
	if (m_showDrivingLines->enabled)
	{
		for (Driver* driver : m_drivingSystem->GetDrivers())
		{
			const Array<DriverPathNode>& path = driver->GetPath();
			if (path.size() > 0)
			{
				for (unsigned int i = 0; i < 1; i++)
					DrawArcs(g, path[i].GetDrivingLine(), Color::MAGENTA);
			}
		}
	}

	// Draw collision debug between drivers
	if (m_showCollisions->enabled)
	{
		for (Driver* driver : m_drivingSystem->GetDrivers())
		{
			for (Driver* other : driver->m_collisions)
			{
				Vector2f arrowDir = other->GetPosition().xy - driver->GetPosition().xy;
				arrowDir.Normalize();
				DrawArrowHead(g, other->GetPosition().xy, arrowDir, r * 2.0f, Color::CYAN);
				g.DrawLine(driver->GetPosition().xy, other->GetPosition().xy, Color::CYAN);
			}
		}
	}

	// Draw hovered-over nodes
	auto m_hoverInfo = m_toolDraw->GetHoverInfo();
	if (m_currentTool == m_toolDraw && m_hoverInfo.subGroup.group != nullptr)
	{
		Vector2f leftEdge = m_hoverInfo.subGroup.group->GetPosition().xy;
		Vector2f right = m_hoverInfo.subGroup.group->GetRightDirection();
		float w = metrics.laneWidth;
		for (int i = 0; i < m_hoverInfo.subGroup.count; i++)
		{
			int index = m_hoverInfo.subGroup.index + i;

			float radius = w * 0.5f;
			Vector2f center = leftEdge + (right * w * (index + 0.5f));
			Color outlineColor = Color::YELLOW;
			if (!m_hoverInfo.isValidSubGroup)
				outlineColor = Color::RED;
			g.FillCircle(center, radius, Color::GRAY);
			g.DrawCircle(center, radius, outlineColor);
		}
	}

	if (m_currentTool == m_toolSelection && m_toolSelection->IsRotatingDirection())
	{
		glBegin(GL_LINES);
		glColor4ubv(Color::RED.data());
		Vector3f center = m_toolSelection->GetRotationCenter();
		glVertex3fv(center.v);
		glVertex3fv(Vector3f(m_cursorGroundPosition.xy, center.z).v);
		glEnd();
	}

	// Draw selection boxes around nodes
	for (NodeGroup* group : m_toolSelection->GetSelection().GetNodeGroups())
	{
		for (int i = 0; i < group->GetNumNodes(); i++)
		{
			Node* node = group->GetNode(i);
			g.DrawCircle(node->GetCenter().xy, node->GetWidth() * 0.6f, Color::GREEN);
		}
	}


	// Draw HUD
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	projection = Matrix4f::CreateOrthographic(0.0f,
		(float)window->GetWidth(), (float)window->GetHeight(),
		0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection.m);

	if (m_currentTool == m_toolSelection)
	{
		if (m_toolSelection->IsCreatingSelection())
			g.DrawRect(m_toolSelection->GetSelectionBox(), Color::GREEN);
	}

	int groupCount = (int)m_network->GetNodeGroups().size();
	int connectionCount = (int)m_network->GetNodeGroupConnections().size();
	int tieCount = (int)m_network->GetNodeGroupTies().size();
	int intersectionCount = (int)m_network->GetIntersections().size();


	String toolName = "(none)";
	if (m_currentTool == m_toolDraw)
		toolName = "Draw Tool";
	if (m_currentTool == m_toolSelection)
		toolName = "Selection Tool";

	using namespace std;
	std::stringstream ss;
	//ss << "FPS: " << GetFPS() << endl;
	ss << "---------------------------" << endl;
	ss << "Tool: " << toolName << endl;
	ss << endl;
	ss << "Groups:        " << groupCount << endl;
	ss << "Connections:   " << connectionCount << endl;
	ss << "Ties:          " << tieCount << endl;
	ss << "Intersections: " << intersectionCount << endl;
	ss << "Drivers:       " << m_drivingSystem->GetDrivers().size() << endl;
	ss << "---------------------------" << endl;

	for (unsigned int i = 0; i < m_debugOptions.size(); i++)
	{
		ostringstream label;
		label << (i + 1) << ". " << m_debugOptions[i]->name << ":";
		ss << std::left << std::setw(18) << label.str() <<
			BOOL2ASCII(m_debugOptions[i]->enabled) << endl;
	}

	ss << "---------------------------" << endl;
	ss << "Traffic: " << int(100 * m_drivingSystem->GetTrafficPercent() + 0.5f) << "%" << endl;

	g.DrawString(m_font, ss.str(), Vector2f(5, 40));

	if (m_currentTool == m_toolDraw)
	{
		for (int i = 0; i < m_toolDraw->GetLaneCount(); i++)
		{
			Vector2f v = Vector2f(10 + (i * 20.0f), 20.0f);
			g.DrawCircle(v, 10, Color::WHITE);
		}
	}

}
