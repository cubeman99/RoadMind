#include "ToolSelection.h"

static HCURSOR hCursorMove;
static HCURSOR hCursorArrow;

ToolSelection::ToolSelection()
{
	hCursorMove = LoadCursor(NULL, IDC_SIZEALL);
	hCursorArrow = LoadCursor(NULL, IDC_ARROW);
}

void ToolSelection::OnBegin()
{
	m_state = State::NONE;
	m_hoverNode = nullptr;
}

void ToolSelection::OnEnd()
{
	Deselect();
	m_state = State::NONE;
	m_hoverNode = nullptr;
	SetCursor(hCursorArrow);
}


void ToolSelection::OnLeftMousePressed()
{
	Vector2f mousePos = GetMousePosition();

	if (m_hoverNode != nullptr)
	{
		if (IsShiftDown())
		{
			if (!m_selection.Contains(m_hoverNode))
				m_selection.Add(m_hoverNode->GetNodeGroup());
			else
				m_selection.Remove(m_hoverNode->GetNodeGroup());

		}
		else if (!m_selection.Contains(m_hoverNode))
		{
			m_selection.Clear();
			m_selection.Add(m_hoverNode->GetNodeGroup());
		}
		m_preMoveInfo.clear();
		for (NodeGroup* group : m_selection.GetNodeGroups())
		{
			PreMoveInfo info;
			info.position = group->GetPosition();
			info.direction = group->GetDirection();
			m_preMoveInfo[group] = info;
		}
		m_state = State::MOVING_SELECTION;
		m_preMoveCursorPosition = mousePos;
	}
	else if (m_state == State::NONE)
	{
		m_state = State::CREATING_SELECTION_BOX;
		m_selectionBoxStart = GetMousePositionInWindow();
	}
}

void ToolSelection::OnLeftMouseReleased()
{
	if (m_state == State::CREATING_SELECTION_BOX)
	{
		SelectMode mode = SelectMode::ADD;
		if (IsAltDown())
			mode = SelectMode::REMOVE;
		if (!IsShiftDown() && !IsAltDown())
			m_selection.Clear();
		Select(m_selectionBox, mode);
		m_state = State::NONE;
	}
	else if (m_state == State::MOVING_SELECTION)
	{
		StopMovement();
	}
	else if (m_state == State::NONE)
	{
	}
}

void ToolSelection::OnRightMousePressed()
{
	if (m_state == State::MOVING_SELECTION)
	{
		CancelMovement();
	}
}

void ToolSelection::OnRightMouseReleased()
{
}

void ToolSelection::Update(float dt)
{
	Vector2f mousePos = GetMousePosition();
	Vector2f mousePosInWindow = GetMousePositionInWindow();
	bool ctrl = IsControlDown();

	m_hoverNode = GetPickedNode();

	if (m_state == State::CREATING_SELECTION_BOX)
	{
		Vector2f mins(
			Math::Min(m_selectionBoxStart.x, mousePosInWindow.x),
			Math::Min(m_selectionBoxStart.y, mousePosInWindow.y));
		Vector2f maxs(
			Math::Max(m_selectionBoxStart.x, mousePosInWindow.x),
			Math::Max(m_selectionBoxStart.y, mousePosInWindow.y));
		m_selectionBox = Rect2f(mins, maxs - mins);
	}
	else if (m_state == State::MOVING_SELECTION)
	{
		Vector2f moveAmount = mousePos - m_preMoveCursorPosition;

		if (ctrl && m_selection.GetNodeGroups().size() == 1)
		{
			for (NodeGroup* group : m_selection.GetNodeGroups())
			{
				if (group->GetTie() != nullptr)
				{
					Vector2f dir = Vector2f::Normalize(
						mousePos - group->GetTie()->GetPosition().xy);
					group->GetTie()->SetDirection(dir *
						Math::Sign(group->GetTie()->GetDirection().Dot(dir)));
				}
				else
					group->SetDirectionFromCenter(Vector2f::Normalize(
					mousePos - group->GetCenterPosition().xy));
			}
		}
		else
		{
			for (NodeGroup* group : m_selection.GetNodeGroups())
			{
				PreMoveInfo info = m_preMoveInfo[group];
				if (group->GetTie() != nullptr)
					group->GetTie()->SetPosition(info.position + Vector3f(moveAmount, 0.0f));
				else
					group->SetPosition(info.position + Vector3f(moveAmount, 0.0f));
			}
		}
	}
	else if (m_state == State::NONE)
	{
	}

	if (m_hoverNode != nullptr && m_selection.Contains(m_hoverNode))
		SetCursor(hCursorMove);
	else
		SetCursor(hCursorArrow);

	if (m_keyboard->IsKeyPressed(Keys::k_delete))
		DeleteSelection();

	// Ctrl+T: Tie/untie node groups
	if (ctrl && m_keyboard->IsKeyPressed(Keys::t))
	{
		if (m_selection.GetNumGroups() == 2)
		{
			int i = 0;
			NodeGroup* groups[2];
			for (NodeGroup* group : m_selection.GetNodeGroups())
				groups[i++] = group;
			if (groups[0]->GetTwin() == groups[1])
			{
				m_network->UntieNodeGroup(groups[0]);
			}
			else
			{
				if (groups[0]->GetTwin() != nullptr)
					m_network->UntieNodeGroup(groups[0]);
				if (groups[1]->GetTwin() != nullptr)
					m_network->UntieNodeGroup(groups[1]);
				m_network->TieNodeGroups(groups[0], groups[1]);
			}
		}
	}

	// Ctrl+D: Deselect
	if (ctrl && m_keyboard->IsKeyPressed(Keys::d))
		Deselect();

	// Ctrl+I: Create Intersection
	if (ctrl && m_keyboard->IsKeyPressed(Keys::i))
	{
		if (m_selection.GetNumGroups() >= 2)
		{
			m_network->CreateIntersection(m_selection.GetNodeGroups());
		}
	}

	// Page-Up/Down: Raise/lower
	float amount = 0.0f;
	if (m_keyboard->IsKeyDown(Keys::page_up))
		amount += 1.0f;
	if (m_keyboard->IsKeyDown(Keys::page_down))
		amount -= 1.0f;
	if (amount != 0.0f)
	{
		amount *= 10.0f * dt;
		for (NodeGroup* group : m_selection.GetNodeGroups())
		{
			IPosition* posObject = (group->GetTie() != nullptr ?
				(IPosition*) group->GetTie() : (IPosition*) group);
			Vector3f pos = posObject->GetPosition();
			pos.z = Math::Max(0.0f, pos.z + amount);
			posObject->SetPosition(pos);
		}
	}
}


Node* ToolSelection::GetPickedNode()
{
	Vector2f cursorPos = GetMousePosition();

	// Get the node group the cursor is currently hovering over
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		// Check if any node in the group is hovered over
		for (int index = 0; index < group->GetNumNodes(); index++)
		{
			Node* node = group->GetNode(index);
			float radius = node->GetWidth() * 0.5f;
			Vector2f center = node->GetCenter().xy;
			if (cursorPos.DistTo(center) <= radius)
			{
				return node;
			}
		}
	}

	return nullptr;
}


bool ToolSelection::IsCreatingSelection() const
{
	return (m_state == State::CREATING_SELECTION_BOX);
}

bool ToolSelection::IsRotatingDirection() const
{
	return (m_state == State::MOVING_SELECTION && IsControlDown());
}

Vector3f ToolSelection::GetRotationCenter() const
{
	if (m_selection.GetNumGroups() == 1)
	{
		NodeGroup* group = *m_selection.GetNodeGroups().begin();
		
		if (group->GetTie() != nullptr)
			return group->GetTie()->GetPosition();
		else
			return group->GetCenterPosition();
	}
	else
	{
		return Vector3f::ZERO;
	}
}

Rect2f ToolSelection::GetSelectionBox() const
{
	return m_selectionBox;
}

NodeGroupSelection& ToolSelection::GetSelection()
{
	return m_selection;
}

void ToolSelection::CancelMovement()
{
	for (NodeGroup* group : m_selection.GetNodeGroups())
	{
		PreMoveInfo info = m_preMoveInfo[group];
		group->SetPosition(info.position);
	}
	m_state = State::NONE;
}

void ToolSelection::StopMovement()
{
	m_state = State::NONE;
}

void ToolSelection::Deselect()
{
	m_selection.Clear();
	m_state = State::NONE;
}

void ToolSelection::DeleteSelection()
{
	for (NodeGroup* group : m_selection.GetNodeGroups())
		m_network->DeleteNodeGroup(group);
	m_selection.Clear();
}

void ToolSelection::Select(const Rect2f& box, SelectMode mode)
{
	Vector2f windowSize((float) m_window->GetWidth(),
		(float) m_window->GetHeight());

	// Get the node group the cursor is currently hovering over
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		// Check if any node in the group is hovered over
		for (int index = 0; index < group->GetNumNodes(); index++)
		{
			Node* node = group->GetNode(index);
			float radius = node->GetWidth() * 0.5f;
			Vector3f center = node->GetCenter();
			Vector3f screenPos = m_camera->GetViewProjectionMatrix() * center;
			Vector2f windowPos(
				(screenPos.x + 1.0f) * 0.5f * windowSize.x,
				(-screenPos.y + 1.0f) * 0.5f * windowSize.y);

			if (m_selectionBox.Contains(windowPos))
			{
				if (mode == SelectMode::ADD)
					m_selection.Add(group);
				else if (mode == SelectMode::REMOVE)
					m_selection.Remove(group);
				break;
			}
		}
	}
}

