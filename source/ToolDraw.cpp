#include "ToolDraw.h"



static Vector2f GetAutoDirection(const Vector2f& from,
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



ToolDraw::ToolDraw()
{
	m_dragInfo.nodeGroup = nullptr;
	m_dragInfo.inputGroup = nullptr;
	m_dragInfo.connection = nullptr;
	m_dragInfo.state = DragState::NONE;

	m_hoverInfo.subGroup.group = nullptr;

	m_laneCount = 1;
}

int ToolDraw::GetLaneCount() const
{
	return m_laneCount;
}


void ToolDraw::CancelDragging()
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

void ToolDraw::StopDragging()
{
	m_dragInfo.nodeGroup = nullptr;
	m_dragInfo.inputGroup = nullptr;
	m_dragInfo.connection = nullptr;
	m_dragInfo.state = DragState::NONE;
}

void ToolDraw::OnBegin()
{
}

void ToolDraw::OnEnd()
{
	CancelDragging();
	m_hoverInfo.subGroup.group = nullptr;
}

void ToolDraw::OnLeftMousePressed()
{
	Vector2f cursorPos = GetMousePosition();

	if (m_dragInfo.state == DragState::NONE)
	{
		m_dragInfo.nodeGroup = m_network->CreateNodeGroup(cursorPos,
			Vector2f::UNITX, m_laneCount);
		m_dragInfo.inputGroup = nullptr;
		m_dragInfo.connection = nullptr;

		if (m_hoverInfo.subGroup.group != nullptr)
		{
			// Begin extending an existing node group
			NodeSubGroup startSubGroup = m_hoverInfo.subGroup;
			m_network->GrowNodeGroup(startSubGroup);
			NodeSubGroup endSubGroup(m_dragInfo.nodeGroup, 0, m_laneCount);
			m_dragInfo.inputGroup = m_hoverInfo.subGroup.group;
			m_dragInfo.connection = m_network->ConnectNodeSubGroups(
				startSubGroup, endSubGroup);
		}
		else
		{
			// Begin dragging a new node group
			m_dragInfo.inputGroup = m_dragInfo.nodeGroup;
			m_dragInfo.nodeGroup = m_network->CreateNodeGroup(cursorPos,
				Vector2f::UNITX, m_laneCount);
			m_dragInfo.connection = m_network->ConnectNodeGroups(
				m_dragInfo.inputGroup, m_dragInfo.nodeGroup);
		}

		m_dragInfo.state = DragState::POSITION;
	}
	else if (m_dragInfo.state == DragState::POSITION)
	{
		if (m_snapInfo.subGroup.group != nullptr)
		{
			if (m_snapInfo.reverse)
			{
				// Connect to the opposide side of an existing node
				m_network->GrowNodeGroup(m_snapInfo.subGroup);
				m_dragInfo.connection->m_output = m_snapInfo.subGroup;
				m_network->TieNodeGroups(m_dragInfo.nodeGroup, m_hoverInfo.subGroup.group);

				m_dragInfo.inputGroup = m_dragInfo.nodeGroup;
				m_dragInfo.nodeGroup = m_network->CreateNodeGroup(cursorPos,
					Vector2f::UNITX, m_snapInfo.subGroup.count);
				m_dragInfo.connection = m_network->ConnectNodeGroups(
					m_dragInfo.inputGroup, m_dragInfo.nodeGroup);
				m_dragInfo.state = DragState::POSITION;
			}
			else
			{
				// Connect to an existing node groups
				m_network->GrowNodeGroup(m_snapInfo.subGroup);
				m_network->ConnectNodeSubGroups(
					m_dragInfo.connection->GetInput(),
					m_snapInfo.subGroup);
				m_network->DeleteNodeGroup(m_dragInfo.nodeGroup);
				m_dragInfo.nodeGroup = nullptr;
				m_dragInfo.state = DragState::NONE;
			}
		}
		else
		{
			// Place the created node group and begin extending it
			m_dragInfo.inputGroup = m_dragInfo.nodeGroup;
			m_dragInfo.nodeGroup = m_network->CreateNodeGroup(cursorPos,
				Vector2f::UNITX, m_dragInfo.inputGroup->GetNumNodes());
			m_dragInfo.connection = m_network->ConnectNodeGroups(
				m_dragInfo.inputGroup, m_dragInfo.nodeGroup);
			m_dragInfo.state = DragState::POSITION;
		}
	}
}

void ToolDraw::OnRightMousePressed()
{
	if (m_dragInfo.state == DragState::POSITION)
	{
		CancelDragging();
	}
}

void ToolDraw::OnLeftMouseReleased()
{
}

void ToolDraw::OnRightMouseReleased()
{
}

void ToolDraw::Update(float dt)
{
	UpdateHoverInfo();
	UdpateDragging(dt);

	
	if (m_keyboard->IsKeyPressed(Keys::add_keypad))
	{
		m_laneCount++;
		if (m_dragInfo.state != DragState::NONE)
		{
			m_network->AddNodesToGroup(m_dragInfo.nodeGroup, 1);
			m_dragInfo.connection->m_output.count++;
		}
	}
	if (m_keyboard->IsKeyPressed(Keys::minus_keypad))
	{
		m_laneCount = Math::Max(1, m_laneCount - 1);
		if (m_dragInfo.state != DragState::NONE)
		{
			if (m_dragInfo.nodeGroup->GetNumNodes() > 1)
				m_network->RemoveNodeFromGroup(m_dragInfo.nodeGroup, 1);
		}
	}
}

NodeGroup* ToolDraw::GetPickedNodeGroup()
{
	Vector2f cursorPos = GetMousePosition();

	// Get the node group the cursor is currently hovering over
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		// If dragging, then don't allow hovering over the draged group or its
		// connected group
		if (m_dragInfo.state != DragState::NONE &&
			(group == m_dragInfo.nodeGroup ||
			group == m_dragInfo.inputGroup))
			continue;

		// Check if any node in the group is hovered over
		for (int index = 0; index < group->GetNumNodes(); index++)
		{
			Node* node = group->GetNode(index);
			float radius = node->GetWidth() * 0.5f;
			Vector2f center = node->GetCenter();
			if (cursorPos.DistTo(center) <= radius)
			{
				return group;
			}
		}
	}

	return nullptr;
}

void ToolDraw::UpdateHoverInfo()
{
	Vector2f cursorPos = GetMousePosition();

	if (!IsShiftDown())
	{
		m_hoverInfo.nodeIndex = 0;
		m_hoverInfo.nodePartialIndex = 0.0f;
		m_hoverInfo.reverse = false;
		m_hoverInfo.subGroup.group = GetPickedNodeGroup();
	}

	if (m_hoverInfo.subGroup.group != nullptr)
	{
		float groupWidth = m_hoverInfo.subGroup.group->GetWidth();
		Vector2f leftEdge = m_hoverInfo.subGroup.group->GetPosition();
		Vector2f rightEdge = m_hoverInfo.subGroup.group->GetRightPosition();
		Vector2f right = m_hoverInfo.subGroup.group->GetDirection();
		right = Vector2f(-right.y, right.x);
		float w = m_network->GetMetrics().laneWidth;
		float offset = (cursorPos.Dot(right) - leftEdge.Dot(right));

		if (offset <= 0.0f)
		{
			m_hoverInfo.nodePartialIndex = offset / w;
			m_hoverInfo.nodeIndex = (int) (offset / w) - 1;
			m_hoverInfo.center = leftEdge +
				(right * w * (m_hoverInfo.nodePartialIndex + 0.5f));
		}
		else if (offset >= groupWidth)
		{
			m_hoverInfo.nodePartialIndex =
				m_hoverInfo.subGroup.group->GetNumNodes() +
				(offset - groupWidth) / w;
			m_hoverInfo.nodeIndex = m_hoverInfo.subGroup.group->GetNumNodes() +
				(int) m_hoverInfo.nodePartialIndex;
			m_hoverInfo.center = leftEdge +
				(right * w * (m_hoverInfo.nodePartialIndex + 0.5f));
		}
		else
		{
			float o = 0.0f;
			for (int index = 0;
				index < m_hoverInfo.subGroup.group->GetNumNodes(); index++)
			{
				Node* node = m_hoverInfo.subGroup.group->GetNode(index);
				if (offset >= o && offset <= o + node->GetWidth())
				{
					m_hoverInfo.nodeIndex = index;
					m_hoverInfo.nodePartialIndex =
						index + ((offset - o) / node->GetWidth());
					m_hoverInfo.center = node->GetCenter();
					break;
				}
				o += node->GetWidth();
			}
		}

		m_hoverInfo.reverse = false;
		m_hoverInfo.subGroup.count = m_laneCount;
		m_hoverInfo.subGroup.index = (int) Math::Floor(
			(m_hoverInfo.nodePartialIndex -
			(m_hoverInfo.subGroup.count - 1) * 0.5f));

		// Check if this proposed connection would interfere with another
		m_hoverInfo.isValidSubGroup = true;
		for (auto connection : m_hoverInfo.subGroup.group->GetOutputs())
		{
			int overlap = NodeSubGroup::GetOverlap(
				m_hoverInfo.subGroup, connection->GetInput());
			if (overlap > 1)
			{
				m_hoverInfo.isValidSubGroup = false;
				break;
			}
		}
	}
}

void ToolDraw::UdpateDragging(float dt)
{
	Vector2f m_mousePosition = GetMousePosition();
	//Keyboard* keyboard = GetKeyboard();
	//Mouse* mouse = GetMouse();
	//MouseState mouseState = mouse->GetMouseState();
	bool ctrl = IsControlDown();
	bool shift = IsShiftDown();
	//bool snapped = false;
	//bool snappedReverse = false;

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

			// Automatically set the direction of the dragged node
			if (m_dragInfo.inputGroup != nullptr)
			{
				Vector2f startCenter =
					m_dragInfo.connection->GetInput().GetLeftPosition();
				Vector2f startRight = m_dragInfo.connection->GetInput().group->GetDirection();
				startRight = Vector2f(-startRight.y, startRight.x);
				startCenter += startRight * width * 0.5f;
				Vector2f autoDirection = GetAutoDirection(startCenter,
					m_dragInfo.inputGroup->GetDirection(), m_mousePosition);
				m_dragInfo.nodeGroup->SetDirection(autoDirection);
			}

			// Attempt to snap to another node
			m_snapInfo.subGroup.group = nullptr;
			if (m_hoverInfo.subGroup.group != nullptr)
			{
				m_snapInfo.reverse = (m_hoverInfo.subGroup.group->GetDirection().Dot(
					m_dragInfo.inputGroup->GetDirection()) < 0.0f);
				if (m_snapInfo.reverse)
				{
					m_snapInfo.subGroup.index = -(m_hoverInfo.subGroup.index +
						m_hoverInfo.subGroup.count);
					m_snapInfo.subGroup.index = Math::Max(0, m_snapInfo.subGroup.index);
					m_snapInfo.subGroup.count = m_dragInfo.connection->GetOutput().count;
					m_snapInfo.subGroup.group = m_dragInfo.nodeGroup;
					m_dragInfo.nodeGroup->SetDirection(
						-m_hoverInfo.subGroup.group->GetDirection());
				}
				else
				{
					m_snapInfo.subGroup = m_hoverInfo.subGroup;
					m_dragInfo.nodeGroup->SetDirection(
						m_hoverInfo.subGroup.group->GetDirection());
				}
				float w = m_network->GetMetrics().laneWidth;
				m_dragInfo.nodeGroup->SetPosition(m_hoverInfo.subGroup.group->GetPosition() +
					(right * w * (float) m_snapInfo.subGroup.index));
			}
		}

		if (m_snapInfo.subGroup.group == nullptr)
		{
			m_dragInfo.nodeGroup->SetPosition(
				m_dragInfo.position - (right * width * 0.5f));
		}
	}
}
