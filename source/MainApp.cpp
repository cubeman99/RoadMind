#include "MainApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>

static HCURSOR  hCursorHand;
static HCURSOR  hCursorArrow;


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
	m_dragNode = nullptr;
	m_hoverInfo.node = nullptr;
	m_dragState = DragState::NONE;
	m_wireframeMode = false;

	m_backgroundTexture = nullptr;
	m_lastNodeGroup = nullptr;
	//Texture::LoadTexture(

	m_rightLaneCount = 1;
	m_leftLaneCount = 1;

	m_p1 = Vector2f(100, 400);
	m_t1 = Vector2f(1, 0);
	m_p2 = Vector2f(300, 390);
	m_t2 = Vector2f(0, 1);
	m_w = 200;
	m_offset = 20;

	m_defaultCameraState.m_viewHeight = 50.0f;
	m_defaultCameraState.m_position = Vector2f::ZERO;
	m_defaultCameraState.m_rotation = 0.0f;
	m_defaultCameraState.m_aspectRatio = GetWindow()->GetAspectRatio();
	m_camera = m_defaultCameraState;

	hCursorHand = LoadCursor(NULL, IDC_SIZEALL);
	hCursorArrow = LoadCursor(NULL, IDC_ARROW);

	m_editMode = EditMode::CREATE;
	m_dragInfo.state = DragState::NONE;
	m_dragInfo.inputGroup = nullptr;
	m_dragInfo.nodeGroup = nullptr;

	//Vector2f screenSize(
	//	(float) GetWindow()->GetWidth(),
	//	(float) GetWindow()->GetHeight());
	//m_camera.m_position = screenSize * 0.5f;
	//m_camera.m_viewHeight = screenSize.y;

	//CreateTestNetwork();
}

void MainApp::CreateTestNetwork()
{
	float w = 48.0f;
	Node* n1a = m_network->CreateNode(Vector2f(100, 101), Vector2f(1, 0), w);
	Node* n2a = m_network->CreateNode(Vector2f(300, 100), Vector2f(1, 0), w);
	Node* n3a = m_network->CreateNode(Vector2f(500, 300), Vector2f(0, 1), w);
	Node* n4a = m_network->CreateNode(Vector2f(501, 400), Vector2f(0, 1), w);
	m_network->Connect(n1a, n2a);
	m_network->Connect(n2a, n3a);
	m_network->Connect(n3a, n4a);

	Node* n1b = m_network->CreateNode(Vector2f(100, 101 + w), Vector2f(1, 0), w);
	Node* n2b = m_network->CreateNode(Vector2f(300, 100 + w), Vector2f(1, 0), w);
	Node* n3b = m_network->CreateNode(Vector2f(500 - w, 300), Vector2f(0, 1), w);
	Node* n4b = m_network->CreateNode(Vector2f(501 - w, 400), Vector2f(0, 1), w);
	m_network->Connect(n1b, n2b);
	m_network->Connect(n2b, n3b);
	m_network->Connect(n3b, n4b);
	m_network->SewNode(n1b, n1a);
	m_network->SewNode(n2b, n2a);
	m_network->SewNode(n3b, n3a);
	m_network->SewNode(n4b, n4a);

	Node* m1a = m_network->CreateNode(Vector2f(100, 101), Vector2f(-1, 0), 48.0f);
	Node* m2a = m_network->CreateNode(Vector2f(300, 100), Vector2f(-1, 0), 48.0f);
	Node* m3a = m_network->CreateNode(Vector2f(500, 300), Vector2f(0, -1), 96.0f);
	Node* m4a = m_network->CreateNode(Vector2f(501, 400), Vector2f(0, -1), 96.0f);
	m_network->Connect(m4a, m3a);
	m_network->Connect(m3a, m2a);
	m_network->Connect(m2a, m1a);
	m_network->SewOpposite(n1a, m1a);
	m_network->SewOpposite(n2a, m2a);
	m_network->SewOpposite(n3a, m3a);
	m_network->SewOpposite(n4a, m4a);
	//m_network->SewOpposite(m1, n1);
	//m_network->SewOpposite(m2, n2);
}


//-----------------------------------------------------------------------------
// Selection
//-----------------------------------------------------------------------------

Node* MainApp::BeginDraggingNewNode(const Vector2f& position)
{
	m_dragNode = m_network->CreateNode();
	m_dragNode->SetCenterPosition(position);
	BeginDragging(m_dragNode, nullptr);
	m_dragState = DragState::DIRECTION;
	return m_dragNode;
}

void MainApp::BeginDragging(Node* node, Node* input)
{
	m_dragInput = input;
	m_dragNode = node;
	m_dragState = DragState::POSITION;
	m_snapInfo.node = nullptr;
	m_snapInfo.reverse = false;
	m_snapInfo.side = LaneSide::NONE;
}

Node* MainApp::BeginExtending(Node* node, LaneSide side, bool reverse)
{
	if (side != LaneSide::NONE)
	{
		float offset = node->GetWidth();
		if (side == LaneSide::LEFT)
			offset = -offset;
		m_dragNode = m_network->CreateNode();
		m_dragNode->SetWidth(node->GetWidth());
		Vector2f direction = node->GetEndNormal();
		if (reverse)
			direction = -direction;
		m_dragNode->SetEndNormal(direction);
		m_dragNode->SetCenterPosition(node->GetCenter() + offset * node->GetEndTangent());

		if (reverse)
			m_network->SewOpposite(node, m_dragNode);
		else if (side == LaneSide::LEFT)
			m_network->SewNode(node, m_dragNode);
		else if (side == LaneSide::RIGHT)
			m_network->SewNode(m_dragNode, node);
		node = m_dragNode;
	}

	m_dragNode = m_network->CreateNode();
	m_dragNode->SetLeftPosition(node->GetLeftEdge());
	m_dragNode->SetEndNormal(node->GetEndNormal());
	m_dragNode->SetWidth(node->GetWidth());
	m_dragNode->SetLeftEdgeTangent(node->GetLeftEdgeTangent());
	m_dragNode->SetRightEdgeTangent(node->GetRightEdgeTangent());
	m_network->Connect(node, m_dragNode);

	m_dragState = DragState::POSITION;
	BeginDragging(m_dragNode, node);
	return m_dragNode;
}

void MainApp::CancelDragging()
{
	if (m_dragInfo.state != DragState::NONE)
	{
		m_network->DeleteNodeGroupConnection(m_dragInfo.connection);
		m_network->DeleteNodeGroup(m_dragInfo.nodeGroup);
		m_dragInfo.nodeGroup = nullptr;
		m_dragInfo.inputGroup = nullptr;
		m_dragInfo.connection = nullptr;
		m_dragInfo.state = DragState::NONE;
	}
}

void MainApp::StopDragging()
{
	m_dragInfo.nodeGroup = nullptr;
	m_dragInfo.inputGroup = nullptr;
	m_dragInfo.connection = nullptr;
	m_dragInfo.state = DragState::NONE;
}

static Vector2f GetAutoNormal(const Vector2f& from,
	const Vector2f& direction, const Vector2f& to)
{
	Vector2f tangent = (to - from).Normalize();
	Vector2f normal(-tangent.y, tangent.x);
	Vector2f middle = (from + to) * 0.5f;
	Vector2f intersection = Line2f::GetLineIntersection(from, from + direction,
		middle, middle + normal);
	Vector2f autoNormal = Vector2f::Normalize(to - intersection);
	float dot = to.Dot(direction) - from.Dot(direction);
	if (Math::Abs(dot) < 0.001f)
		return -direction;
	else if (dot < 0)
		return -autoNormal;
	else
		return autoNormal;
}

void MainApp::UpdateDragging()
{
	Window* window = GetWindow();
	Keyboard* keyboard = GetKeyboard();
	Mouse* mouse = GetMouse();
	MouseState mouseState = mouse->GetMouseState();
	bool ctrl = keyboard->IsKeyDown(Keys::left_control) ||
		keyboard->IsKeyDown(Keys::right_control);
	bool shift = keyboard->IsKeyDown(Keys::left_shift) ||
		keyboard->IsKeyDown(Keys::right_shift);
	int scroll = mouseState.z - mouse->GetPrevMouseState().z;
	bool snapped = false;

	if (m_dragInfo.state != DragState::NONE)
	{
		if (m_dragInfo.state == DragState::POSITION)
		{
			float width = m_dragInfo.nodeGroup->GetWidth();
			Vector2f right = m_dragInfo.nodeGroup->GetDirection();
			right = Vector2f(-right.y, right.x);

			if (ctrl)
			{
				Vector2f v = m_mousePosition - m_dragInfo.position;
				if (v.Length() > 0.00001f)
				{
					m_dragInfo.nodeGroup->SetDirection(Vector2f::Normalize(v));
					right = m_dragInfo.nodeGroup->GetDirection();
					right = Vector2f(-right.y, right.x);
				}
			}
			else
			{
				m_dragInfo.position = m_mousePosition;

				if (m_dragInfo.inputGroup != nullptr)
				{
					Vector2f startCenter =
						m_dragInfo.connection->GetInput().GetLeftPosition();
					Vector2f startRight = m_dragInfo.connection->GetInput().group->GetDirection();
					startRight = Vector2f(-startRight.y, startRight.x);
					startCenter += startRight * width * 0.5f;
					Vector2f autoNormal = GetAutoNormal(startCenter,
						m_dragInfo.inputGroup->GetDirection(), m_mousePosition);
					m_dragInfo.nodeGroup->SetDirection(autoNormal);
				}

				if (m_hoverInfo.node != nullptr)
				{
					// Snap
					snapped = true;
					m_dragInfo.nodeGroup->SetPosition(
						m_hoverInfo.nodeGroup->GetNode(m_hoverInfo.startIndex)->GetPosition());
					m_dragInfo.nodeGroup->SetDirection(
						m_hoverInfo.nodeGroup->GetDirection());
				}
			}

			if (!snapped)
			{
				m_dragInfo.nodeGroup->SetPosition(
					m_dragInfo.position - (right * width * 0.5f));
			}

			if (mouse->IsButtonPressed(MouseButtons::right))
				CancelDragging();
			else if (mouse->IsButtonPressed(MouseButtons::left))
			{
				m_dragInfo.inputGroup = m_dragInfo.nodeGroup;
				m_dragInfo.nodeGroup = m_network->CreateNodeGroup(m_mousePosition,
					Vector2f::UNITX, m_rightLaneCount);
				m_dragInfo.connection = m_network->ConnectNodeGroups(
					m_dragInfo.inputGroup, m_dragInfo.nodeGroup);
				m_dragInfo.state = DragState::POSITION;
			}
		}
	}
	else
	{
		if (mouse->IsButtonPressed(MouseButtons::left))
		{
			m_dragInfo.nodeGroup = m_network->CreateNodeGroup(m_mousePosition,
				Vector2f::UNITX, m_rightLaneCount);
			m_dragInfo.inputGroup = nullptr;
			m_dragInfo.connection = nullptr;

			if (m_hoverInfo.node != nullptr)
			{
				NodeSubGroup startSubGroup;
				startSubGroup.group = m_hoverInfo.nodeGroup;
				startSubGroup.index = m_hoverInfo.startIndex;
				startSubGroup.count = Math::Min(m_rightLaneCount,
					m_hoverInfo.nodeGroup->GetNumNodes() - m_hoverInfo.startIndex);
				NodeSubGroup endSubGroup(m_dragInfo.nodeGroup, 0, m_rightLaneCount);
				m_dragInfo.inputGroup = m_hoverInfo.node->GetNodeGroup();
				m_dragInfo.connection = m_network->ConnectNodeSubGroups(
					startSubGroup, endSubGroup);
			}
			else
			{
				m_dragInfo.inputGroup = m_dragInfo.nodeGroup;
				m_dragInfo.nodeGroup = m_network->CreateNodeGroup(m_mousePosition,
					Vector2f::UNITX, m_rightLaneCount);
				m_dragInfo.connection = m_network->ConnectNodeGroups(
					m_dragInfo.inputGroup, m_dragInfo.nodeGroup);
			}

			m_dragInfo.state = DragState::POSITION;
		}
	}
	return;

	if (m_dragState == DragState::POSITION)
	{
		if (ctrl)
		{
			// Update direction
			Vector2f center = m_dragNode->GetCenter();
			Vector2f direction = m_mousePosition - center;
			if (direction.Length() > 0.0f)
			{
				direction = Vector2f::Normalize(direction);
				m_dragNode->SetEndNormal(direction);
				m_dragNode->SetLeftEdgeTangent(direction);
				m_dragNode->SetRightEdgeTangent(direction);
			}
			m_dragNode->SetCenterPosition(center);
		}
		else
		{
			m_dragNode->SetCenterPosition(m_mousePosition);

			// Automatically choose and end normal
			if (m_dragNode->GetNumInputs() > 0)
			{
				Node* prev = (*m_dragNode->GetInputs().begin())->GetInput();
				Vector2f tangent = (m_mousePosition -
					prev->GetCenter()).Normalize();
				Vector2f normal(-tangent.y, tangent.x);
				Vector2f middle = (m_mousePosition + prev->GetCenter()) * 0.5f;
				Vector2f intersection = Line2f::GetLineIntersection(
					prev->GetCenter(), prev->GetCenter() + prev->GetEndNormal(),
					middle, middle + normal);
				Vector2f autoNormal = Vector2f::Normalize(m_mousePosition - intersection);
				float dot = m_mousePosition.Dot(prev->GetEndNormal()) - prev->GetCenter().Dot(prev->GetEndNormal());
				if (Math::Abs(dot) < 0.001f)
					autoNormal = -prev->GetEndNormal();
				else if (dot < 0)
					autoNormal = -autoNormal;
				m_dragNode->SetEndNormal(autoNormal);
				m_dragNode->SetLeftEdgeTangent(autoNormal);
				m_dragNode->SetRightEdgeTangent(autoNormal);
			}
		}

		m_snapInfo.node = nullptr;
		if (m_hoverInfo.node != nullptr)
		{
			snapped = true;

			m_snapInfo.node = m_hoverInfo.node;
			m_snapInfo.side = m_hoverInfo.side;
			m_snapInfo.center = m_hoverInfo.center;

			Vector2f hoverCenter = m_hoverInfo.node->GetCenter();
			if (m_hoverInfo.side != LaneSide::NONE)
			{
				float offset = m_hoverInfo.node->GetWidth();
				if (m_hoverInfo.side == LaneSide::LEFT)
					offset = -offset;
				hoverCenter += m_hoverInfo.node->GetEndTangent() * offset;
			}

			Vector2f direction = m_hoverInfo.node->GetEndNormal();
			if (m_dragInput != nullptr &&
				m_dragInput->GetEndNormal().Dot(direction) < 0)
			{
				m_snapInfo.reverse = true;
				direction = -direction;
			}
			else
			{
				m_snapInfo.reverse = false;
			}

			m_dragNode->SetEndNormal(direction);
			m_dragNode->SetWidth(m_hoverInfo.node->GetWidth());
			m_dragNode->SetCenterPosition(hoverCenter);
		}

		if (mouse->IsButtonPressed(MouseButtons::right))
		{
			m_network->DeleteNode(m_dragNode);
			m_dragNode = nullptr;
			m_dragState = DragState::NONE;
		}
		else if (mouse->IsButtonPressed(MouseButtons::left))
		{
			if (m_snapInfo.node != nullptr && m_dragNode->GetNumInputs() > 0)
			{
				if (m_hoverInfo.side != LaneSide::NONE)
				{
					// Sew the drag node to another node
					Vector2f direction = m_snapInfo.node->GetEndNormal();
					if (m_snapInfo.reverse)
						direction = -direction;
					m_dragNode->SetWidth(m_snapInfo.node->GetWidth());
					m_dragNode->SetEndNormal(direction);
					m_dragNode->SetCenterPosition(m_snapInfo.center);
					if (m_snapInfo.reverse)
						m_network->SewOpposite(m_dragNode, m_snapInfo.node);
					else if (m_snapInfo.side == LaneSide::RIGHT)
						m_network->SewNode(m_dragNode, m_snapInfo.node);
					else if (m_snapInfo.side == LaneSide::LEFT)
						m_network->SewNode(m_snapInfo.node, m_dragNode);
					m_dragState = DragState::NONE;
					BeginExtending(m_dragNode);
				}
				else
				{
					// Delete the node and connect two nodes instead
					Node* output = m_hoverInfo.node;
					Node* input = (*m_dragNode->GetInputs().begin())->GetInput();
					m_network->Disconnect(input, m_dragNode);
					m_network->Connect(input, output);
					m_network->DeleteNode(m_dragNode);
					m_dragNode = nullptr;
					m_dragState = DragState::NONE;
				}
			}
			else if (ctrl)
			{
				m_dragState = DragState::NONE;
				BeginExtending(m_dragNode);
			}
			else
			{
				m_dragState = DragState::DIRECTION;
			}
		}
	}
	else if (m_dragState == DragState::DIRECTION)
	{
		Vector2f center = m_dragNode->GetCenter();
		Vector2f direction = m_mousePosition - center;
		if (direction.Length() > 4.0f)
		{
			direction = Vector2f::Normalize(direction);
			m_dragNode->SetEndNormal(direction);
			m_dragNode->SetLeftEdgeTangent(direction);
			m_dragNode->SetRightEdgeTangent(direction);
		}
		m_dragNode->SetCenterPosition(center);

		if (mouse->IsButtonPressed(MouseButtons::right))
		{
			m_network->DeleteNode(m_dragNode);
			m_dragNode = nullptr;
			m_dragState = DragState::NONE;
		}
		else if (!mouse->IsButtonDown(MouseButtons::left))
		{
			m_dragState = DragState::NONE;
			BeginExtending(m_dragNode);
		}
	}
	/*else if (mouse->IsButtonPressed(MouseButtons::left))
	{
	if (m_hoverInfo.node == nullptr)
	BeginDraggingNewNode(m_mousePosition);
	else if (ctrl)
	BeginDragging(m_hoverInfo.node);
	else
	{
	bool reverse = (m_mousePosition.Dot(m_hoverInfo.node->GetEndNormal()) <
	m_hoverInfo.node->GetCenter().Dot(m_hoverInfo.node->GetEndNormal()));
	BeginExtending(m_hoverInfo.node, m_hoverInfo.side, reverse);
	}
	}*/
}

void MainApp::UpdateHoverInfo()
{
	m_hoverInfo.node = nullptr;
	m_hoverInfo.nodeGroup = nullptr;
	m_hoverInfo.nodeIndex = 0;
	m_hoverInfo.nodePartialIndex = 0.0f;
	m_hoverInfo.side = LaneSide::NONE;
	m_hoverInfo.reverse = false;

	// Check which node is being hovered over
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		if (m_dragInfo.state != DragState::NONE &&
			m_dragInfo.nodeGroup == group)
			continue;

		for (int index = 0; index < group->GetNumNodes(); index++)
		{
			Node* node = group->GetNode(index);

			float radius = node->GetWidth() * 0.5f;
			Vector2f center = node->GetCenter();
			Vector2f right = node->GetEndTangent();
			Vector2f leftEdge = center - (right * radius);
			Vector2f rightEdge = center + (right * radius);

			float t = (m_mousePosition.Dot(right) - leftEdge.Dot(right))
				/ (rightEdge.Dot(right) - leftEdge.Dot(right));

			if (m_mousePosition.DistTo(center) <= radius)
			{
				m_hoverInfo.nodeGroup = group;
				m_hoverInfo.node = node;
				m_hoverInfo.nodeIndex = index;
				m_hoverInfo.nodePartialIndex = (float) index + t;
				m_hoverInfo.side = LaneSide::NONE;
				m_hoverInfo.center = node->GetCenter();
				m_hoverInfo.reverse = false;
				m_hoverInfo.startIndex = Math::Max(0,
					(int) ((m_hoverInfo.nodePartialIndex - m_rightLaneCount * 0.5f) + 0.5f));
			}
			//else if (node->GetLeftNode() == nullptr &&
			//	m_mousePosition.DistTo(leftCenter) <= radius)
			//{
			//	m_hoverInfo.node = node;
			//	m_hoverInfo.side = LaneSide::LEFT;
			//	m_hoverInfo.center = leftCenter;
			//}
			//else if (node->GetRightNode() == nullptr &&
			//	m_mousePosition.DistTo(rightCenter) <= radius)
			//{
			//	m_hoverInfo.node = node;
			//	m_hoverInfo.side = LaneSide::RIGHT;
			//	m_hoverInfo.center = rightCenter;
			//}
		}
	}
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

static void DrawArcs(Graphics2D& g, const BiarcPair& arcs, const Color& color)
{
	DrawArc(g, arcs.first, color);
	DrawArc(g, arcs.second, color);
}

void MainApp::DrawNode(Graphics2D& g, Node* node)
{
	Vector2f leftEdge = node->GetLeftEdge();
	Vector2f rightEdge = node->GetRightEdge();
	Color laneEdgeColor = Color::BLACK;
	Vector2f direction = node->GetEndNormal();
	RoadMetrics metrics = *node->GetMetrics();

	Color lineColor = Color::WHITE;
	Color fillColor(255, 128, 32);
	Color outputColor(255, 128, 32);
	Color inputColor(32, 128, 255);

	bool laneEnd = (node->GetNumInputs() == 0 ||
		node->GetNumOutputs() == 0);
	if (laneEnd)
	{
		if (node->GetNumInputs() == 0)
			fillColor = outputColor;
		else
			fillColor = inputColor;

		if (m_hoverInfo.node == node)
			fillColor = Color::Lerp(fillColor, Color::WHITE, 0.5f);
		if (!laneEnd)
			g.DrawLine(leftEdge, rightEdge, Color::BLACK);

		RoadMetrics metrics = m_network->GetMetrics();
		float width = metrics.laneWidth * 0.2f;
		float tipLength = metrics.laneWidth * 0.16f;
		float tipWidth = metrics.laneWidth * 0.3f;

		Vector2f endNormal = node->GetEndNormal();
		if (node->GetNumInputs() == 0)
		{
			endNormal = -endNormal;
			tipLength = -tipLength;
		}
		Vector2f center1 = node->GetCenter();
		Vector2f center2 = center1 - (endNormal * width);

		Vector2f left2 = Line2f::GetLineIntersection(
			leftEdge, leftEdge + node->GetLeftEdgeTangent(),
			center2, center2 + node->GetEndTangent());
		Vector2f right2 = Line2f::GetLineIntersection(
			rightEdge, rightEdge + node->GetRightEdgeTangent(),
			center2, center2 + node->GetEndTangent());

		Vector2f connecterTip = center1 + endNormal * tipLength;
		Vector2f connecterLeft = center1 - node->GetEndTangent() * tipWidth * 0.5f;
		Vector2f connecterRight = center1 + node->GetEndTangent() * tipWidth * 0.5f;

		// Draw stop line
		if (node->GetNumOutputs() == 0)
		{
			glBegin(GL_TRIANGLE_FAN);
			glColor4ubv(Color::WHITE.data());
			glVertex2fv(node->GetLeftEdge().v);
			glVertex2fv((node->GetLeftEdge() - direction * metrics.stopLineWidth).v);
			glVertex2fv((node->GetRightEdge() - direction * metrics.stopLineWidth).v);
			glVertex2fv(node->GetRightEdge().v);
			glEnd();
		}

		// Draw the buckle
		if (m_showDebug)
		{
			if (node->GetNumInputs() == 0)
			{
				glBegin(GL_TRIANGLE_FAN);
				glColor4ubv(fillColor.data());
				glVertex2fv(left2.v);
				glVertex2fv(leftEdge.v);
				glVertex2fv(connecterLeft.v);
				glVertex2fv(connecterTip.v);
				glVertex2fv(center2.v);
				glEnd();
				glBegin(GL_TRIANGLE_FAN);
				glColor4ubv(fillColor.data());
				glVertex2fv(center2.v);
				glVertex2fv(connecterTip.v);
				glVertex2fv(connecterRight.v);
				glVertex2fv(rightEdge.v);
				glVertex2fv(right2.v);
				glEnd();
			}
			else
			{
				glBegin(GL_TRIANGLE_FAN);
				glColor4ubv(fillColor.data());
				glVertex2fv(left2.v);
				glVertex2fv(leftEdge.v);
				glVertex2fv(rightEdge.v);
				glVertex2fv(right2.v);
				glEnd();
				glBegin(GL_TRIANGLE_FAN);
				glColor4ubv(fillColor.data());
				glVertex2fv(connecterLeft.v);
				glVertex2fv(connecterTip.v);
				glVertex2fv(connecterRight.v);
				glEnd();
			}

			glBegin(GL_LINE_LOOP);
			glColor4ubv(Color::WHITE.data());
			glVertex2fv(left2.v);
			glVertex2fv(leftEdge.v);
			glVertex2fv(connecterLeft.v);
			glVertex2fv(connecterTip.v);
			glVertex2fv(connecterRight.v);
			glVertex2fv(rightEdge.v);
			glVertex2fv(right2.v);
			glEnd();
		}
	}
	if (m_showDebug)
	{
		g.DrawLine(leftEdge, rightEdge, Color::GRAY);
		g.DrawLine(node->GetCenter(),
			node->GetCenter() + node->GetEndNormal() * node->GetWidth() * 0.2f, Color::GRAY);

		if (m_hoverInfo.node == node)
		{
			Vector2f hoverCenter = node->GetCenter();

			if (m_hoverInfo.side != LaneSide::NONE)
			{
				float offset = node->GetWidth();
				if (m_hoverInfo.side == LaneSide::LEFT)
					offset = -offset;
				hoverCenter += node->GetEndTangent() * offset;
			}

			g.FillCircle(hoverCenter, node->GetWidth() * 0.5f, Vector4f(1, 1, 1, 0.3f));
			if (m_hoverInfo.side != LaneSide::NONE)
				g.DrawCircle(hoverCenter, node->GetWidth() * 0.5f, Color::WHITE);
			g.DrawLine(hoverCenter, node->GetCenter(), Color::RED);
		}
		g.DrawCircle(node->GetCenter(), node->GetWidth() * 0.5f, Color::WHITE);
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

	if (keyboard->IsKeyPressed(Keys::i))
		m_wireframeMode = !m_wireframeMode;

	// R: Clear road network
	if (keyboard->IsKeyPressed(Keys::r))
	{
		StopDragging();
		m_lastNodeGroup = nullptr;
		m_dragState = DragState::NONE;
		m_network->ClearNodes();
		m_camera.m_position = m_defaultCameraState.m_position;
		m_camera.m_rotation = m_defaultCameraState.m_rotation;
	}
	if (keyboard->IsKeyPressed(Keys::t))
	{
		m_dragState = DragState::NONE;
		m_network->ClearNodes();
		CreateTestNetwork();
	}
	if (keyboard->IsKeyPressed(Keys::enter) && m_hoverInfo.node != nullptr)
	{
		Driver* driver = new Driver(m_network, m_hoverInfo.node);
		m_drivers.push_back(driver);
	}

	for (Driver* driver : m_drivers)
		driver->Update(dt);


	// D: Toggle debug display
	if (ctrl && keyboard->IsKeyPressed(Keys::d))
		m_showDebug = !m_showDebug;

	if (GetMouse()->IsButtonDown(MouseButtons::left))
	{
		if (ctrl)
			m_t1 = Vector2f::Normalize(m_mousePosition - m_p1);
		else
			m_p1 = m_mousePosition;
	}
	if (GetMouse()->IsButtonDown(MouseButtons::right))
	{
		if (ctrl)
			m_t2 = Vector2f::Normalize(m_mousePosition - m_p2);
		else
			m_p2 = m_mousePosition;
	}
	if (keyboard->IsKeyDown(Keys::space))
	{
		m_w = m_mousePosition.x;
	}
	if (keyboard->IsKeyDown(Keys::e))
	{
		m_offset = m_mousePosition.x;
	}

	UpdateCameraControls(dt);
	UpdateHoverInfo();

	if (m_hoverInfo.node != nullptr)
	{
		SetCursor(hCursorHand);
		m_hoverInfo.reverse = (m_mousePosition.Dot(
			m_hoverInfo.node->GetEndNormal()) <
			m_hoverInfo.node->GetCenter().Dot(m_hoverInfo.node->GetEndNormal()));
	}
	else
	{
		SetCursor(hCursorArrow);
	}

	if (keyboard->IsKeyPressed(Keys::k_delete) &&
		m_hoverInfo.node != nullptr && m_dragState == DragState::NONE)
	{
		m_network->RemoveNodeFromGroup(m_hoverInfo.node->GetNodeGroup(), 1);
		//m_network->DeleteNode(m_hoverInfo.node);
		//m_hoverInfo.node = nullptr;
	}

	if (keyboard->IsKeyPressed(Keys::add_keypad))
	{
		m_rightLaneCount++;
		if (m_dragInfo.state != DragState::NONE)
		{
			m_network->AddNodesToGroup(m_dragInfo.nodeGroup, 1);
			m_dragInfo.connection->m_output.count++;
		}
		else if (m_hoverInfo.node != nullptr)
			m_network->AddNodesToGroup(m_hoverInfo.node->GetNodeGroup(), 1);
	}
	if (keyboard->IsKeyPressed(Keys::minus_keypad))
	{
		m_rightLaneCount = Math::Max(1, m_rightLaneCount - 1);
		if (m_dragInfo.state != DragState::NONE)
		{
			if (m_dragInfo.nodeGroup->GetNumNodes() > 1)
				m_network->RemoveNodeFromGroup(m_dragInfo.nodeGroup, 1);
		}
		else if (m_hoverInfo.node != nullptr)
		{
			if (m_hoverInfo.node->GetNodeGroup()->GetNumNodes() > 1)
				m_network->RemoveNodeFromGroup(m_hoverInfo.node->GetNodeGroup(), 1);
		}
	}

	UpdateDragging();

	/*
	if (mouse->IsButtonPressed(MouseButtons::left))
	{
	NodeGroup* group = m_network->CreateNodeGroup(m_mousePosition,
	Vector2f::UNITX, m_rightLaneCount);

	if (m_hoverInfo.node != nullptr)
	{
	NodeGroupTie* tie = m_network->TieNodeGroups(group, m_hoverInfo.node->GetNodeGroup());
	}
	else if (m_lastNodeGroup != nullptr)
	{
	Vector2f tangent = (m_mousePosition -
	m_lastNodeGroup->GetPosition()).Normalize();
	Vector2f normal(-tangent.y, tangent.x);
	Vector2f middle = (m_mousePosition + m_lastNodeGroup->GetPosition()) * 0.5f;
	Vector2f intersection = Line2f::GetLineIntersection(
	m_lastNodeGroup->GetPosition(), m_lastNodeGroup->GetPosition() + m_lastNodeGroup->GetDirection(),
	middle, middle + normal);
	Vector2f autoNormal = Vector2f::Normalize(m_mousePosition - intersection);
	float dot = m_mousePosition.Dot(m_lastNodeGroup->GetDirection()) -
	m_lastNodeGroup->GetPosition().Dot(m_lastNodeGroup->GetDirection());
	if (Math::Abs(dot) < 0.001f)
	autoNormal = -m_lastNodeGroup->GetDirection();
	else if (dot < 0)
	autoNormal = -autoNormal;
	group->SetDirection(autoNormal);
	m_network->ConnectNodeGroups(m_lastNodeGroup, group);
	}
	m_lastNodeGroup = group;
	}*/

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
	//for (Connection* connection : m_network->GetConnections())
	//	DrawNodeGroupConnection(g, connection);
	//for (Connection* connection : m_network->GetConnections())
	//	DrawRoadMarkings(g, connection);
	//for (Node* node : m_network->GetNodes())
	//DrawNode(g, node);

	//Biarc arc1, arc2;
	//Vector2f p1 = m_p1;
	//Vector2f t1 = m_t1;
	//Vector2f p2 = m_p2;
	//Vector2f t2 = m_t2;
	//Vector2f pm, q1, q2;
	//BiarcType type;
	//ComputeBiarcs(p1, t1, p2, t2, pm, q1, q2, arc1, arc2, type);
	//DrawArc(g, arc1, Color::GREEN);
	//DrawArc(g, arc2, Color::GREEN);
	//float width = m_magic;
	//float halfWidth = width * 0.5f;
	////DrawArc(g, CreateParallelBiarc(arc1, halfWidth), Color::DARK_GREEN);
	////DrawArc(g, CreateParallelBiarc(arc2, halfWidth), Color::DARK_YELLOW);
	//float t = arc1.length / (arc1.length + arc2.length);


	/*
	t = 0.9f;
	DrawArc(g, CreateParallelBiarc(arc1, 0.0f, (2 * -halfWidth) * t), Color::DARK_GREEN);
	//DrawArc(g, CreateParallelBiarc(arc2, (2 * -halfWidth) * t, (2 * -halfWidth)), Color::DARK_YELLOW);
	DrawArc(g, CreateParallelBiarc(arc2.Reverse(), (2 * halfWidth), (2 * halfWidth) * t), Color::DARK_YELLOW);

	DrawArc(g, CreateParallelBiarc(arc1, 0.0f, (2 * halfWidth) * t), Color::DARK_GREEN);
	//DrawArc(g, CreateParallelBiarc(arc2, (2 * -halfWidth) * t, (2 * -halfWidth)), Color::DARK_YELLOW);
	DrawArc(g, CreateParallelBiarc(arc2.Reverse(), (2 * -halfWidth), (2 * -halfWidth) * t), Color::DARK_YELLOW);

	{
	glBegin(GL_LINE_STRIP);
	glColor4ubv(Color::GREEN.data());
	float t = arc1.length / (arc1.length + arc2.length);
	t = 0.9f;
	//DrawArc(g, CreateParallelBiarc(arc1, 0.0f, (2 * -halfWidth) * t), Color::DARK_GREEN);
	////DrawArc(g, CreateParallelBiarc(arc2, (2 * -halfWidth) * t, (2 * -halfWidth)), Color::DARK_YELLOW);
	//DrawArc(g, CreateParallelBiarc(arc2.Reverse(), (2 * halfWidth), (2 * halfWidth) * t), Color::DARK_YELLOW);
	//DrawArc(g, CreateParallelBiarc(arc1, 0.0f, (2 * halfWidth) * t), Color::DARK_GREEN);
	////DrawArc(g, CreateParallelBiarc(arc2, (2 * -halfWidth) * t, (2 * -halfWidth)), Color::DARK_YELLOW);
	//DrawArc(g, CreateParallelBiarc(arc2.Reverse(), (2 * -halfWidth), (2 * -halfWidth) * t), Color::DARK_YELLOW);

	glEnd();
	}
	*/


	//DrawPoint(g, arc1.start, Color::RED);
	//DrawPoint(g, arc1.end, Color::RED);
	//DrawPoint(g, arc2.end, Color::RED);
	//DrawPoint(g, arc1.center, Color::CYAN);
	//DrawPoint(g, arc2.center, Color::CYAN);
	//g.DrawLine(arc1.start, arc1.center, Color::CYAN);
	//g.DrawLine(arc1.end, arc1.center, Color::CYAN);
	//g.DrawLine(arc2.start, arc2.center, Color::CYAN);
	//g.DrawLine(arc2.end, arc2.center, Color::CYAN);
	//DrawPoint(g, q1, Color::BLUE);
	//DrawPoint(g, q2, Color::BLUE);
	//g.DrawLine(q1, p1, Color::BLUE);
	//g.DrawLine(q1, pm, Color::BLUE);
	//g.DrawLine(q2, p2, Color::BLUE);
	//g.DrawLine(q2, pm, Color::BLUE);
	////DrawPoint(g, pm, Color::RED);
	//halfWidth = m_magic;
	//halfWidth = m_w;

	////for (halfWidth = 1.0f; halfWidth < 400.0f; halfWidth += 2)
	//{
	//	Vector2f n2(-t2.y, t2.x);
	//	Vector2f p3 = p2 - (halfWidth * n2);
	//	float w = 32.0f;
	//	//t = (m_magic / window->GetWidth());
	//	
	//	Vector2f midPointLeftNormal = (arc1.end - arc1.center) / arc1.radius;
	//	if (arc1.angle < 0.0f)
	//		midPointLeftNormal = -midPointLeftNormal;
	//	Biarc testArc1, testArc2;
	//	ComputeExpandingBiarcs(p1, t1, p2, t2, arc1.end,
	//		midPointLeftNormal, halfWidth, testArc1, testArc2);
	//	//Vector2f passThrough = arc1.end + midPointLeftNormal * m_offset;
	//	//DrawPoint(g, passThrough, Color::YELLOW);
	//	//Biarc testArc1 = ComputeArc(p1, t1, passThrough);
	//	//Biarc testArc2 = ComputeArc(p3, -t2, passThrough);
	//	DrawArc(g, testArc1, Color::YELLOW);
	//	DrawArc(g, testArc2, Color::YELLOW);
	//	g.DrawLine(testArc1.center, testArc1.end, Color::MAGENTA);
	//	g.DrawLine(testArc2.center, testArc1.end, Color::MAGENTA);

	//	float bestT = 0.0f;
	//	float bestDot = -99;
	//	for (float t = 0.0f; t <= 1.0f; t += 0.01f)
	//	{
	//		w = t * halfWidth;
	//		Vector2f midpoint = arc1.end + Vector2f::Normalize(arc1.center - arc2.center) * w;
	//		//DrawPoint(g, midpoint, Color::GREEN);
	//		//DrawPoint(g, p3, Color::GREEN);

	//		Vector2f c1 = ComputeCircleCenter(p1, t1, midpoint);
	//		float r1 = c1.DistTo(p1);
	//		//DrawPoint(g, c1, Color::YELLOW);
	//		//g.DrawCircle(c1, r1, Color::YELLOW);

	//		Vector2f c2 = ComputeCircleCenter(p3, -t2, midpoint);
	//		float r2 = c2.DistTo(p3);
	//		//DrawPoint(g, c2, Color::YELLOW);
	//		//g.DrawCircle(c2, r2, Color::YELLOW);

	//		//g.DrawLine(c1, midpoint, Color::YELLOW);
	//		//g.DrawLine(c2, midpoint, Color::YELLOW);

	//		float dot = Vector2f::Normalize(c1 - midpoint).Dot(Vector2f::Normalize(midpoint - c2));
	//		if (dot > bestDot)
	//		{
	//			bestDot = dot;
	//			bestT = t;
	//		}
	//	}
	//	t = bestT;


	//	{
	//		w = t * halfWidth;
	//		Vector2f midpoint = arc1.end + Vector2f::Normalize(arc1.center - arc2.center) * w;

	//		//DrawPoint(g, Vector2f(halfWidth, window->GetHeight() - w * 3), Color::MAGENTA);
	//		//DrawPoint(g, Vector2f(halfWidth, window->GetHeight() - t * 400), Color::GREEN);
	//		//DrawPoint(g, Vector2f(halfWidth, window->GetHeight() - 400), Color::GREEN);

	//		if (m_showDebug)
	//		{
	//			DrawPoint(g, midpoint, Color::GREEN);
	//			DrawPoint(g, p3, Color::GREEN);

	//			Vector2f c1 = ComputeCircleCenter(p1, t1, midpoint);
	//			float r1 = c1.DistTo(p1);
	//			DrawPoint(g, c1, Color::YELLOW);
	//			//g.DrawCircle(c1, r1, Color::YELLOW);
	//			DrawArc(g, ComputeArc(p1, t1, midpoint), Color::YELLOW);

	//			Vector2f c2 = ComputeCircleCenter(p3, -t2, midpoint);
	//			float r2 = c2.DistTo(p3);
	//			DrawPoint(g, c2, Color::YELLOW);
	//			//g.DrawCircle(c2, r2, Color::YELLOW);
	//			DrawArc(g, ComputeArc(p3, -t2, midpoint), Color::YELLOW);
	//			g.DrawLine(p1, c1, Color::MAGENTA);
	//			g.DrawLine(p3, c2, Color::MAGENTA);

	//			float xx = c2.DistTo(arc2.center);
	//			float yy = p3.DistTo(arc2.center) - arc2.radius;

	//			Vector2f midpointNormal = midpoint - c2;
	//			midpointNormal = Vector2f(-midpointNormal.y, midpointNormal.x);
	//			q2 = Line2f::GetLineIntersection(p3, p3 + t2, midpoint, midpoint + midpointNormal);
	//			q1 = Line2f::GetLineIntersection(p1, p1 + t1, midpoint, midpoint + midpointNormal);
	//			DrawPoint(g, q2, Color::MAGENTA);
	//			DrawPoint(g, q1, Color::MAGENTA);
	//			g.DrawLine(q1, p1, Color::MAGENTA);
	//			g.DrawLine(q1, midpoint, Color::MAGENTA);
	//			g.DrawLine(q2, p3, Color::MAGENTA);
	//			g.DrawLine(q2, midpoint, Color::MAGENTA);
	//				

	//			g.DrawLine(c1, midpoint, Color::MAGENTA);
	//			g.DrawLine(c2, midpoint, Color::MAGENTA);
	//		}
	//	}
	//}

	//halfWidth = 200;
	//p2.x += halfWidth;
	//ComputeBiarcs(p1, t1, p2, t2, arc1, arc2);
	//DrawArc(g, arc1, Color::MAGENTA);
	//DrawArc(g, arc2, Color::CYAN);

	RoadMetrics metrics = m_network->GetMetrics();

	// Draw road surfaces
	Color colorReadFill = Color(64, 64, 64);
	for (NodeGroupConnection* surface : m_network->GetNodeGroupConnections())
	{
		BiarcPair leftEdge = surface->GetLeftEdgeLine();
		BiarcPair rightEdge = surface->GetRightEdgeLine();
		int count = 10;
		count = (int) (Math::Max(leftEdge.Length(), rightEdge.Length()) / 1.5f + 0.5f);
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

	// Draw road markings
	for (NodeGroupConnection* surface : m_network->GetNodeGroupConnections())
	{
		DrawArcs(g, surface->m_dividerLines[0], Color::YELLOW);
		for (unsigned int i = 1; i < surface->m_dividerLines.size(); i++)
			DrawArcs(g, surface->m_dividerLines[i], Color::WHITE);
	}

	// Draw node groups
	float r = 0.2f;
	float r2 = 0.4f;
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		Vector2f center = group->GetPosition();
		g.FillCircle(center, r * 2.0f, Color::RED);
		g.DrawLine(center, center + (group->GetDirection() * r * 3), Color::WHITE);

		for (int i = 0; i < group->GetNumNodes(); i++)
		{
			Node* node = group->GetNode(i);
			g.DrawLine(node->GetLeftEdge(), node->GetRightEdge(), Color::WHITE);
			g.FillCircle(node->GetRightEdge(), r, Color::WHITE);
			if (m_hoverInfo.nodeGroup == group &&
				i >= m_hoverInfo.startIndex &&
				i < m_hoverInfo.startIndex + m_rightLaneCount)
				g.FillCircle(node->GetCenter(), node->GetWidth() * 0.5f, Color::GRAY);
			g.DrawCircle(node->GetCenter(), node->GetWidth() * 0.5f, Color::WHITE);
			g.DrawLine(node->GetCenter(), node->GetCenter() + node->GetEndNormal() * r2, Color::WHITE);
		}
	}

	for (Connection* connection : m_network->GetConnections())
	{
		//g.DrawLine(connection->GetInput()->GetCenter(),
		//	connection->GetOutput()->GetCenter(), Color::WHITE);
		//DrawArrowHead(g, connection->GetOutput()->GetCenter(),
		//	connection->GetOutput()->GetEndNormal(), r2 * 2, Color::WHITE);
	}

	for (Driver* driver : m_drivers)
	{
		float radius = 1.75f * 0.5f;
		g.DrawCircle(driver->GetPosition(), radius, Color::GREEN);
		g.DrawLine(driver->GetPosition(), driver->GetPosition() +
			driver->GetDirection() * radius, Color::BLACK);
	}

	Keyboard* keyboard = GetKeyboard();
	bool ctrl = keyboard->IsKeyDown(Keys::left_control) ||
		keyboard->IsKeyDown(Keys::right_control);
	if (m_dragState == DragState::DIRECTION ||
		(m_dragState == DragState::POSITION && ctrl))
	{
		g.DrawLine(m_dragNode->GetCenter(), m_mousePosition, Color::RED);
	}

	// Draw HUD

	projection = Matrix4f::CreateOrthographic(0.0f,
		(float) window->GetWidth(), (float) window->GetHeight(),
		0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection.m);

	for (int i = 0; i < m_rightLaneCount; i++)
	{
		Vector2f v = Vector2f(30.0f + (i * 20.0f), 30.0f);
		g.DrawCircle(v, 10, Color::WHITE);
	}
}
