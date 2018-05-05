#include "MainApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>


MainApp::MainApp()
{
}

MainApp::~MainApp()
{
	UnloadShaders();
}

void MainApp::OnInitialize()
{
	LoadShaders();

	m_network = new RoadNetwork();

	m_showDebug = true;
	m_wireframeMode = false;

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
	if (m_showDebug)
	{
		g.FillCircle(arcs.first.start, 0.15f, color);
		g.FillCircle(arcs.first.end, 0.15f, color);
		g.FillCircle(arcs.second.end, 0.15f, color);
	}
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

	Color colorReadFill = Color(64, 64, 64);

	// Fill in the road surface
	if (connection->GetVertices(LaneSide::RIGHT).size() ==
		connection->GetVertices(LaneSide::LEFT).size())
	{
		unsigned int vertexCount = connection->GetVertices(LaneSide::RIGHT).size();
		glBegin(GL_TRIANGLE_STRIP);
		glColor4ubv(colorReadFill.data());
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

	Color colorReadFill = Color(64, 64, 64);
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

	// I: Wireframe mode
	if (keyboard->IsKeyPressed(Keys::i))
		m_wireframeMode = !m_wireframeMode;

	// F1: Toggle debug display
	if (keyboard->IsKeyPressed(Keys::f1))
		m_showDebug = !m_showDebug;

	// R: Clear road network
	if (keyboard->IsKeyPressed(Keys::r))
	{
		m_toolDraw->CancelDragging();
		m_network->ClearNodes();
		m_camera.m_position = m_defaultCameraState.m_position;
		m_camera.m_rotation = m_defaultCameraState.m_rotation;
	}

	// T: Create test network
	if (keyboard->IsKeyPressed(Keys::t))
	{
		m_toolDraw->CancelDragging();
		m_network->ClearNodes();
		CreateTestNetwork();
	}

	// M: Selection tool
	if (keyboard->IsKeyPressed(Keys::m))
		SetTool(m_toolSelection);
	// P: Draw tool
	if (keyboard->IsKeyPressed(Keys::p))
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

	Vector3f gridCenter;
	gridCenter.z = 0.0f;
	gridCenter.xy = m_camera.m_position;
	Meters gridRadius = m_camera.m_viewHeight;
	DrawGridFloor(gridCenter, 1.0f, gridRadius);

	if (m_wireframeMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	RoadMetrics metrics = m_network->GetMetrics();

	// Draw road surfaces
	Color colorReadFill = Color(64, 64, 64);
	for (NodeGroupConnection* surface : m_network->GetNodeGroupConnections())
	{
		BiarcPair leftEdge = surface->GetLeftEdgeLine();
		BiarcPair rightEdge = surface->GetRightEdgeLine();
		float maxLength = Math::Max(leftEdge.Length(), rightEdge.Length());
		int count = 10;
		float interval = 1.5f;
		count = (int) ((maxLength / interval) + 0.5f);
		count = Math::Max(2, count);
		glBegin(GL_TRIANGLE_STRIP);
		glColor3ubv(colorReadFill.data());
		for (int i = 0; i <= count; i++)
		{
			float t = (float) i / count;
			glVertex2fv(leftEdge.GetPoint(leftEdge.Length() * t).v);
			glVertex2fv(rightEdge.GetPoint(rightEdge.Length() * t).v);
		}
		glEnd();
	}

	// Draw intersection surfaces
	for (RoadIntersection* intersection : m_network->GetIntersections())
	{
		for (NodeGroup* group : intersection->GetNodeGroups())
		{
			for (int i = 0; i < group->GetNumNodes(); i++)
			{
				Node* node = group->GetNode(i);
				if (m_showDebug)
				{
					g.DrawCircle(node->GetCenter(), node->GetWidth() * 0.51f, Color::RED);
				}
			}
		}
	}


	// Draw road markings
	for (NodeGroupConnection* connection : m_network->GetNodeGroupConnections())
	{
		if (connection->m_dividerLines.size() > 0)
		{
			//DrawArcs(g, connection->m_dividerLines.front(), Color::YELLOW * Vector4f(Vector3f::ONE, 0.3f));
			//DrawArcs(g, connection->m_dividerLines.back(), Color::WHITE * Vector4f(Vector3f::ONE, 0.3f));
		}
		if (m_showDebug)
		{
			DrawArcs(g, connection->m_visualShoulderLines[0], Color::GREEN * Vector4f(Vector3f::ONE, 0.3f));
			DrawArcs(g, connection->m_visualShoulderLines[1], Color::GREEN * Vector4f(Vector3f::ONE, 0.3f));
		}

		DrawArcs(g, connection->m_visualEdgeLines[0], Color::YELLOW);
		for (unsigned int i = 1; i < connection->m_dividerLines.size() - 1; i++)
			DrawArcs(g, connection->m_dividerLines[i], Color::WHITE);
		DrawArcs(g, connection->m_visualEdgeLines[1], Color::WHITE);
	}

	float r = 0.2f;
	float r2 = 0.4f;

	// Draw node groups
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		Vector2f center = group->GetPosition();
		if (m_showDebug)
			g.DrawLine(center, center + (group->GetDirection() * r * 3), Color::WHITE);

		for (int i = 0; i < group->GetNumNodes(); i++)
		{
			Node* node = group->GetNode(i);
			if (m_showDebug)
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
		if (m_showDebug)
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
	
	g.DrawString(m_font, ss.str(), Vector2f(5, 5));

	if (m_currentTool == m_toolDraw)
	{
		for (int i = 0; i < m_toolDraw->GetLaneCount(); i++)
		{
			Vector2f v = Vector2f(30.0f + (i * 20.0f), 100.0f);
			g.DrawCircle(v, 10, Color::WHITE);
		}
	}

}
