#include "MainApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>
#include <iomanip>

#define BOOL2ASCII(x) (x ? "TRUE" : "FALSE")

MainApp::MainApp()
{
	m_debugOptions.push_back(m_showRoadMarkings = new DebugOption("Markings", true));
	m_debugOptions.push_back(m_showEdgeLines = new DebugOption("Edges", true));
	m_debugOptions.push_back(m_showRoadSurface = new DebugOption("Surface", true));
	m_debugOptions.push_back(m_wireframeMode = new DebugOption("Wireframe", false));
	m_debugOptions.push_back(m_showDebug = new DebugOption("Debug", true));
	m_debugOptions.push_back(m_showNodes = new DebugOption("Nodes", true));

	//m_showEdgeLines->enabled = true;
	//m_showDebug->enabled = true;
	//m_showRoadMarkings->enabled = false;
	//m_showRoadSurface->enabled = false;
	//m_showNodes->enabled = false;
}

MainApp::~MainApp()
{
	UnloadShaders();
}

void MainApp::OnInitialize()
{
	LoadShaders();

	m_network = new RoadNetwork();

	m_backgroundTexture = nullptr;

	m_defaultCameraState.m_viewHeight = 50.0f;
	m_defaultCameraState.m_position = Vector2f::ZERO;
	m_defaultCameraState.m_rotation = 0.0f;
	m_defaultCameraState.m_aspectRatio = GetWindow()->GetAspectRatio();
	m_camera = m_defaultCameraState;

	m_font = SpriteFont::LoadBuiltInFont(BuiltInFonts::FONT_CONSOLE);

	m_editMode = EditMode::CREATE;

	// Create the editor tools
	m_toolSelection = new ToolSelection();
	m_toolDraw = new ToolDraw();
	m_tools.push_back(m_toolDraw);
	m_tools.push_back(m_toolSelection);
	m_currentTool = nullptr;

	SetTool(m_toolDraw);

	//Vector2f screenSize(
	//	(float) GetWindow()->GetWidth(),
	//	(float) GetWindow()->GetHeight());
	//m_camera.m_position = screenSize * 0.5f;
	//m_camera.m_viewHeight = screenSize.y;

	//CreateTestNetwork();
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
	Vector2f windowSize((float) window->GetWidth(), (float) window->GetHeight());
	Vector2f mousePos((float) mouse->GetMouseState().x, (float) mouse->GetMouseState().y);
	Vector2f mousePosPrev((float) mouse->GetPrevMouseState().x, (float) mouse->GetPrevMouseState().y);
	int scroll = mouse->GetMouseState().z - mouse->GetPrevMouseState().z;
	bool ctrl = keyboard->IsKeyDown(Keys::left_control) ||
		keyboard->IsKeyDown(Keys::right_control);

	if (scroll != 0)
	{
		m_camera.m_viewHeight *= Math::Pow(0.9f, (float) scroll);
	}

	if (!ctrl)
	{
		Vector2f move = Vector2f::ZERO;
		if (keyboard->IsKeyDown(Keys::a))
			move += m_camera.GetLeft();
		if (keyboard->IsKeyDown(Keys::d))
			move -= m_camera.GetLeft();
		if (keyboard->IsKeyDown(Keys::w))
			move += m_camera.GetUp();
		if (keyboard->IsKeyDown(Keys::s))
			move -= m_camera.GetUp();
		move = move.Normalize();
		if (move.LengthSquared() > 0.0f)
			m_camera.m_position += move * (m_camera.m_viewHeight * 0.9f * dt);
	}

	if (ctrl && mouse->IsButtonDown(MouseButtons::right))
	{
		Vector2f windowCenter = windowSize * 0.5f;
		Radians angle = -(mousePos.x - mousePosPrev.x) * 0.003f;

		//float dot = (mousePos - windowCenter).Normalize().Dot(
		//	(mousePosPrev - windowCenter).Normalize());
		//float angle = 0.0f;

		//if (dot >= 1.0f)
		//{
		//	angle = 0.0f;
		//}
		//else if (dot <= -1.0f)
		//{
		//	angle = Math::PI;
		//}
		//else
		//{
		//	angle = Math::ACos(dot);
		//	Vector2f normal = mousePosPrev - windowCenter;
		//	normal = Vector2f(-normal.y, normal.x);
		//	if (mousePos.Dot(normal) < mousePosPrev.Dot(normal))
		//		angle = -angle;
		//}

		m_camera.m_rotation += angle;
	}
}


//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

static void DrawPoint(Graphics2D& g, const Vector2f& point, const Color& color)
{
	g.FillCircle(point, 4, color);
}

static void DrawArc(Graphics2D& g, const Biarc& arc, const Color& color)
{
	if (arc.radius == 0.0f)
	{
		g.DrawLine(arc.start, arc.end, color);
	}
	else
	{
		Vector2f v = arc.start;
		float angle = arc.angle / 10.0f;
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

void MainApp::FillZippedArcs(Graphics2D& g, const Biarc& a, const Biarc& b, const Color& color)
{
	float maxLength = Math::Max(a.length, b.length);
	int count = 10;
	float interval = 1.5f;
	count = (int) ((maxLength / interval) + 0.5f);
	count = Math::Max(2, count);
	glBegin(GL_TRIANGLE_STRIP);
	glColor3ubv(color.data());
	glVertex2fv(a.start.v);
	glVertex2fv(b.start.v);
	for (int i = 1; i <= count; i++)
	{
		float t = (float) i / count;
		glVertex2fv(a.GetPoint(a.length * t).v);
		glVertex2fv(b.GetPoint(b.length * t).v);
	}
	glVertex2fv(a.end.v);
	glVertex2fv(b.end.v);
	glEnd();
}

void MainApp::DrawGridFloor(const Vector3f& center, Meters squareSize, Meters gridRadius)
{
	int majorTick = 10;

	// Snap the grid radius to the square size.
	gridRadius = Math::Ceil(gridRadius / squareSize) * squareSize;

	float startX = center.x - gridRadius;
	float startZ = center.y - gridRadius;
	int indexX = (int) Math::Floor(startX / (squareSize));
	int indexZ = (int) Math::Floor(startZ / (squareSize));
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
		Color color = Color(30, 30, 30);
		if (indexX % majorTick == 0)
			color = Color(80, 80, 80);
		glColor4ubv(color.data());
		//glVertex3f(x, center.y, startZ);
		//glVertex3f(x, center.y, endZ);
		glVertex3f(x, startZ, center.z);
		glVertex3f(x, endZ, center.z);

		// Draw line along x-axis.
		color = Color(30, 30, 30);
		if (indexZ % majorTick == 0)
			color = Color(80, 80, 80);
		glColor4ubv(color.data());
		//glVertex3f(startX, center.y, z);
		//glVertex3f(endX, center.y, z);
		glVertex3f(startX, z, center.z);
		glVertex3f(endX, z, center.z);
	}
	glEnd();
	glDepthMask(true);
}

void MainApp::DrawConnection(Graphics2D& g, Connection* connection)
{
}

void MainApp::DrawNodeGroupConnection(Graphics2D& g, Connection* connection)
{
	MouseState mouseState = GetMouse()->GetMouseState();

	Connection* leftConnection = connection->GetLeftConnection();
	Connection* rightConnection = connection->GetRightConnection();

	Color colorRoadFill = Color(64, 64, 64);

	// Fill in the road surface
	if (connection->GetVertices(LaneSide::RIGHT).size() ==
		connection->GetVertices(LaneSide::LEFT).size())
	{
		unsigned int vertexCount = connection->GetVertices(LaneSide::RIGHT).size();
		glBegin(GL_TRIANGLE_STRIP);
		glColor4ubv(colorRoadFill.data());
		for (unsigned int i = 0; i < vertexCount; i++)
		{
			glVertex2fv(connection->GetVertices(LaneSide::RIGHT)[i].v);
			glVertex2fv(connection->GetVertices(LaneSide::LEFT)[i].v);
		}
		glEnd();
	}
}

void MainApp::DrawRoadMarkings(Graphics2D& g, Connection* connection)
{
	Connection* leftConnection = connection->GetLeftConnection();
	Connection* rightConnection = connection->GetRightConnection();
	float distance = connection->GetDistance();
	RoadMetrics metrics = m_network->GetMetrics();

	Color colorRoadFill = Color(64, 64, 64);
	Color colorSideDividerLine = Color::YELLOW;
	Color colorLaneDividerLine = Color::WHITE;

	// Draw the edges
	for (int s = 0; s < 2; s++)
	{
		LaneSide side = (LaneSide) s;

		if (side == LaneSide::RIGHT && !connection->IsRightMostLane())
			continue; // Don't draw the same divider twice

		const Array<Vector2f>& vertices = connection->GetVertices((LaneSide) side);
		Color color = colorLaneDividerLine;
		if (side == LaneSide::LEFT && connection->IsLeftMostLane())
			color = colorSideDividerLine;
		if (side == LaneSide::CENTER)
			color = Color::GRAY;
		LaneDivider divider = connection->GetLaneDivider(side);

		// Determine the horizontal offset of the divider
		float offset = 0.0f;
		if (side == LaneSide::LEFT && leftConnection != nullptr)
		{
			if (leftConnection->GetLeftConnection() == connection)
			{
				if (leftConnection->GetLaneDivider(LaneSide::LEFT) ==
					LaneDivider::DASHED && divider == LaneDivider::DASHED)
				{
					offset = -metrics.dividerWidth * 0.5f;
					if (connection->GetInput()->GetNodeId() >
						leftConnection->GetInput()->GetNodeId())
						continue; // Don't draw the same divider twice
				}
				else
				{
					offset = metrics.dividerWidth * 0.5f;
				}
			}
			else
				offset = -metrics.dividerWidth * 0.5f;
		}
		if (side == LaneSide::RIGHT && rightConnection != nullptr)
		{
			continue;
		}

		Vector2f v1, v2, v3, v4;

		if (distance < 1000.0f)
		{
			if (divider == LaneDivider::DASHED)
			{
				glBegin(GL_QUADS);
				glColor4ubv(color.data());
				float spacing = metrics.dividerLength + metrics.dividerGapLength;
				for (float d = 0.0f; d < distance; d += spacing)
				{
					float d2 = Math::Min(distance, d + metrics.dividerLength);
					v1 = connection->GetPoint(d, side, offset);
					v2 = connection->GetPoint(d, side,
						offset + metrics.dividerWidth);
					v3 = connection->GetPoint(d2, side, offset);
					v4 = connection->GetPoint(d2, side,
						offset + metrics.dividerWidth);
					glVertex2fv(v1.v);
					glVertex2fv(v2.v);
					glVertex2fv(v4.v);
					glVertex2fv(v3.v);
				}
				glEnd();
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
				glColor4ubv(color.data());
				for (float d = 0.0f; d < distance; d += 0.5f)
				{
					v1 = connection->GetPoint(d, side, offset);
					v2 = connection->GetPoint(d, side,
						offset + metrics.dividerWidth);
					glVertex2fv(v1.v);
					glVertex2fv(v2.v);
				}
				v1 = connection->GetPoint(distance, side, offset);
				v2 = connection->GetPoint(distance, side,
					offset + metrics.dividerWidth);
				glVertex2fv(v1.v);
				glVertex2fv(v2.v);
				glEnd();
			}
		}

		/*glBegin(GL_TRIANGLE_STRIP);
		glColor4ubv(color.data());
		for (float d = 0.0f; d < distance; d += 0.5f)
		{
		v1 = connection->GetPoint(d, 3.75f);
		v2 = connection->GetPoint(d, 3.75f - metrics.dividerWidth);
		glVertex2fv(v1.v);
		glVertex2fv(v2.v);
		}
		glEnd();*/

		/*for (unsigned int i = 0; i < vertices.size(); i++)
		{
		g.DrawCircle(vertices[i], 2, Color::WHITE);
		}*/
	}
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
	for (Driver* driver : m_drivers)
		delete driver;
	m_drivers.clear();

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
	Vector2f windowSize((float) window->GetWidth(), (float) window->GetHeight());
	m_mousePosition.x = (float) mouseState.x;
	m_mousePosition.y = (float) mouseState.y;
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
		m_network->ClearNodes();
		m_camera.m_position = m_defaultCameraState.m_position;
		m_camera.m_rotation = m_defaultCameraState.m_rotation;
	}

	// T: Create test network
	if (!ctrl && keyboard->IsKeyPressed(Keys::t))
	{
		m_toolDraw->CancelDragging();
		m_network->ClearNodes();
		CreateTestNetwork();
	}

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
	//if (keyboard->IsKeyPressed(Keys::enter) && m_hoverInfo.node != nullptr)
	//{
	//	Driver* driver = new Driver(m_network, m_hoverInfo.node);
	//	m_drivers.push_back(driver);
	//}

	// Update the current tool
	if (m_currentTool != nullptr)
	{
		m_currentTool->m_keyboard = GetKeyboard();
		m_currentTool->m_mouse = GetMouse();
		m_currentTool->m_network = m_network;
		m_currentTool->m_mousePosition = m_mousePosition;

		if (mouse->IsButtonPressed(MouseButtons::left))
			m_currentTool->OnLeftMousePressed();
		if (mouse->IsButtonPressed(MouseButtons::right))
			m_currentTool->OnRightMousePressed();
		if (mouse->IsButtonReleased(MouseButtons::left))
			m_currentTool->OnLeftMouseReleased();
		if (mouse->IsButtonReleased(MouseButtons::right))
			m_currentTool->OnRightMouseReleased();

		m_currentTool->Update(dt);
	}

	// Update the drivers
	for (Driver* driver : m_drivers)
		driver->Update(dt);

	UpdateCameraControls(dt);

	m_network->UpdateNodeGeometry();
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
	bool showDebug = false;
	Array<Vector2f> remaining = points;
	unsigned int count = remaining.size();
	unsigned int searched = 0;
	Array<Vector2f> vertices;
	Convexity convexity;
	for (int i = 0; count >= 3 &&
		searched < count; i = (i + 1) % count)
	{
		Vector2f a = remaining[i];
		Vector2f b = remaining[(i + 1) % count];
		Vector2f c = remaining[(i + 2) % count];

		convexity = GetConvexity(a, b, c);

		if (a.DistToSqr(b) < 0.0001f)
		{
			remaining.erase(remaining.begin() + ((i + 1) % count));
			if (i == count - 1)
				i--;
			count--;
			searched = 0;
			if (showDebug)
				g.DrawCircle(points[i], 0.15f, Color::CYAN);
			i--;
		}
		else if (convexity == Convexity::STRAIGHT)
		{
			searched++;
		}
		else
		{
			bool contains = false;
			//for (unsigned int j = 2; j < count - 1; j++)
			for (unsigned int j = 0; j < points.size(); j++)
			{
				//Vector2f v = remaining[(i + j) % count];
				Vector2f v = points[j];
				if (
					v.DistToSqr(a) > 0.00001f &&
					v.DistToSqr(b) > 0.00001f &&
					v.DistToSqr(c) > 0.00001f &&
					IsInsideTriangle(a, b, c, v))
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
	if (searched == count && showDebug)
		glColor4ubv(Color::RED.data());
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (showDebug)
		{
			float t = (i / (float) vertices.size());
			Vector3f c = color.ToVector3f();
			if (searched == count && showDebug)
				c = Color::RED.ToVector3f();
			c *= t;
			glColor3fv(c.data());
		}
		glVertex2fv(vertices[i].v);
	}
	glEnd();

	if (showDebug)
	{
		for (unsigned int i = 0; i < points.size(); i++)
		{
			float t = (i / (float) points.size());
			Color c = Color::YELLOW.ToVector3f() * t;
			g.FillCircle(points[i], 0.1f, c);
		}
		if (remaining.size() >= 3)
		{
			for (unsigned int i = 0; i < remaining.size(); i++)
				g.DrawCircle(remaining[i], 0.12f, Color::RED);
		}
	}
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
			int count = (int) ((Math::Abs(arc.angle) / Math::TWO_PI) * 50) + 2;
			//float angle = arc.angle / count;
			//v.Rotate(arc.center, angle);
			for (int j = 1; j < count; j++)
			{
				float t = j / (float) count;
				float angle = arc.angle * t;
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


void MainApp::OnRender()
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

	//m_camera.m_aspectRatio = GetWindow()->GetAspectRatio();
	//Matrix4f projection = Matrix4f::CreateOrthographic(0.0f, window->GetWidth(), window->GetHeight(), 0.0f, -1.0f, 1.0f);
	Matrix4f projection = Matrix4f::IDENTITY;
	Matrix4f view = m_camera.GetWorldToCameraMatrix();
	//view = Matrix4f::IDENTITY;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((projection * view).m);

	if (m_backgroundTexture != nullptr)
	{
		g.DrawTexture(m_backgroundTexture,
			m_backgroundPosition - (m_backgroundSize * 0.5f));
	}

	Color colorEdgeLines = Color::WHITE;
	
	float r = 0.2f;
	float r2 = 0.4f;
	Color c = Color::WHITE;

	Vector3f gridCenter;
	gridCenter.z = 0.0f;
	gridCenter.xy = m_camera.m_position;
	Meters gridRadius = m_camera.m_viewHeight;
	DrawGridFloor(gridCenter, 1.0f, gridRadius);

	if (m_wireframeMode->enabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	RoadMetrics metrics = m_network->GetMetrics();

	// Draw road surfaces
	if (m_showRoadSurface->enabled)
	{
		// Draw lane surfaces
		Color colorRoadFill = Color(64, 64, 64);
		Random::Seed(0);
		int index = 0;
		for (NodeGroupConnection* surface : m_network->GetNodeGroupConnections())
		{
			BiarcPair leftEdge = surface->GetLeftVisualShoulderLine();
			BiarcPair rightEdge = surface->GetRightVisualShoulderLine();

			Color c(Vector3f(Random::NextFloat(0.5f, 1.0f),
				Random::NextFloat(0.5f, 1.0f),
				Random::NextFloat(0.5f, 1.0f)));

			auto seamsIL = surface->GetSeams(IOType::INPUT, LaneSide::LEFT);
			auto seamsIR = surface->GetSeams(IOType::INPUT, LaneSide::RIGHT);
			auto seamsOL = surface->GetSeams(IOType::OUTPUT, LaneSide::LEFT);
			auto seamsOR = surface->GetSeams(IOType::OUTPUT, LaneSide::RIGHT);

			Array<Biarc> surfaceContour;
			for (auto it = seamsIR.rbegin(); it != seamsIR.rend(); it++)
			{
				surfaceContour.push_back(it->second.Reverse());
				surfaceContour.push_back(it->first.Reverse());
			}
			for (auto it = seamsIL.begin(); it != seamsIL.end(); it++)
			{
				surfaceContour.push_back(it->first);
				surfaceContour.push_back(it->second);
			}
			surfaceContour.push_back(leftEdge.first);
			surfaceContour.push_back(leftEdge.second);
			for (auto it = seamsOL.begin(); it != seamsOL.end(); it++)
			{
				surfaceContour.push_back(it->first);
				surfaceContour.push_back(it->second);
			}
			for (auto it = seamsOR.rbegin(); it != seamsOR.rend(); it++)
			{
				surfaceContour.push_back(it->second.Reverse());
				surfaceContour.push_back(it->first.Reverse());
			}
			surfaceContour.push_back(rightEdge.second.Reverse());
			surfaceContour.push_back(rightEdge.first.Reverse());

			FillShape(g, surfaceContour, colorRoadFill);
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
	int index = 0;
	for (NodeGroupConnection* connection : m_network->GetNodeGroupConnections())
	{
		// Draw shoudler edges
		if (m_showEdgeLines->enabled)
		{
			DrawArcs(g, connection->m_visualShoulderLines[0], colorEdgeLines);
			DrawArcs(g, connection->m_visualShoulderLines[1], colorEdgeLines);

			// Draw seams
			for (BiarcPair seam : connection->GetSeams(IOType::INPUT, LaneSide::LEFT))
				DrawArcs(g, seam, Color::MAGENTA);
			for (BiarcPair seam : connection->GetSeams(IOType::INPUT, LaneSide::RIGHT))
				DrawArcs(g, seam, Color::MAGENTA);
			for (BiarcPair seam : connection->GetSeams(IOType::OUTPUT, LaneSide::LEFT))
				DrawArcs(g, seam, Color::MAGENTA);
			for (BiarcPair seam : connection->GetSeams(IOType::OUTPUT, LaneSide::RIGHT))
				DrawArcs(g, seam, Color::MAGENTA);
		}
		index++;

		// Draw lane divider lines
		if (m_showRoadMarkings->enabled)
		{
			DrawArcs(g, connection->m_visualEdgeLines[0], Color::YELLOW);
			for (unsigned int i = 1; i < connection->m_dividerLines.size() - 1; i++)
				DrawArcs(g, connection->m_dividerLines[i], Color::WHITE);
			DrawArcs(g, connection->m_visualEdgeLines[1], Color::WHITE);
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

	// Draw node groups
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		Vector2f center = group->GetPosition();

		for (int i = 0; i < group->GetNumNodes(); i++)
		{
			Node* node = group->GetNode(i);

			// Draw stop line at end of lane
			if (m_showRoadMarkings->enabled && group->GetOutputs().empty())
				g.DrawLine(node->GetLeftEdge(), node->GetRightEdge(), Color::WHITE);

			if (m_showNodes->enabled)
			{
				g.DrawLine(node->GetLeftEdge(), node->GetRightEdge(), Color::WHITE);
				g.FillCircle(node->GetRightEdge(), r, Color::WHITE);
				//if (m_hoverInfo.nodeGroup == group &&
				//i >= m_hoverInfo.startIndex &&
				//i < m_hoverInfo.startIndex + m_rightLaneCount)
				//g.FillCircle(node->GetCenter(), node->GetWidth() * 0.5f, Color::GRAY);
				g.DrawCircle(node->GetCenter(), node->GetWidth() * 0.5f, Color::WHITE);
				g.DrawLine(node->GetCenter(), node->GetCenter() +
					node->GetDirection() * node->GetWidth() * 0.5f,
					Color::WHITE);
			}
		}
	}

	// Draw node ties
	for (NodeGroupTie* tie : m_network->GetNodeGroupTies())
	{
		if (m_showDebug->enabled)
		{
			g.FillCircle(tie->GetPosition(), r * 2.0f, Color::RED);
		}
	}

	for (Driver* driver : m_drivers)
	{
		float radius = 1.75f * 0.5f;
		g.DrawCircle(driver->GetPosition(), radius, Color::GREEN);
		g.DrawLine(driver->GetPosition(), driver->GetPosition() +
			driver->GetDirection() * radius, Color::BLACK);
	}

	// Draw hovered-over nodes
	auto m_hoverInfo = m_toolDraw->GetHoverInfo();
	if (m_hoverInfo.subGroup.group != nullptr)
	{
		Vector2f leftEdge = m_hoverInfo.subGroup.group->GetPosition();
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

	// Draw selection boxes around nodes
	for (NodeGroup* group : m_toolSelection->GetSelection().GetNodeGroups())
	{
		for (int i = 0; i < group->GetNumNodes(); i++)
		{
			Node* node = group->GetNode(i);
			g.DrawCircle(node->GetCenter(), node->GetWidth() * 0.6f, Color::GREEN);
		}
	}


	if (m_currentTool == m_toolSelection)
	{
		if (m_toolSelection->IsCreatingSelection())
			g.DrawRect(m_toolSelection->GetSelectionBox(), Color::GREEN);
	}


	Keyboard* keyboard = GetKeyboard();
	bool ctrl = keyboard->IsKeyDown(Keys::left_control) ||
		keyboard->IsKeyDown(Keys::right_control);
	//if (m_dragState == DragState::DIRECTION ||
	//	(m_dragState == DragState::POSITION && ctrl))
	//{
	//	g.DrawLine(m_dragNode->GetCenter(), m_mousePosition, Color::RED);
	//}

	// Draw HUD
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	projection = Matrix4f::CreateOrthographic(0.0f,
		(float) window->GetWidth(), (float) window->GetHeight(),
		0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection.m);

	int groupCount = (int) m_network->GetNodeGroups().size();
	int connectionCount = (int) m_network->GetNodeGroupConnections().size();
	int tieCount = (int) m_network->GetNodeGroupTies().size();
	int intersectionCount = (int) m_network->GetIntersections().size();


	String toolName = "(none)";
	if (m_currentTool == m_toolDraw)
		toolName = "Draw Tool";
	if (m_currentTool == m_toolSelection)
		toolName = "Selection Tool";

	using namespace std;
	std::stringstream ss;
	ss << "Tool: " << toolName << endl;
	ss << endl;
	ss << "Groups:        " << groupCount << endl;
	ss << "Connections:   " << connectionCount << endl;
	ss << "Ties:          " << tieCount << endl;
	ss << "Intersections: " << intersectionCount << endl;
	ss << "---------------------" << endl;

	for (unsigned int i = 0; i < m_debugOptions.size(); i++)
	{
		ostringstream label;
		label << (i + 1) << ". " << m_debugOptions[i]->name << ":";
		ss << std::left << std::setw(15) << label.str() <<
			BOOL2ASCII(m_debugOptions[i]->enabled) << endl;
	}

	g.DrawString(m_font, ss.str(), Vector2f(5, 5));

	if (m_currentTool == m_toolDraw)
	{
		for (int i = 0; i < m_toolDraw->GetLaneCount(); i++)
		{
			Vector2f v = Vector2f(200 + (i * 20.0f), 20.0f);
			g.DrawCircle(v, 10, Color::WHITE);
		}
	}

}
