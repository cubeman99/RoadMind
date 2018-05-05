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
	
	if (m_hoverNode != nullptr && m_selection.Contains(m_hoverNode))
	{
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
		m_selectionBoxStart = mousePos;
	}
}

void ToolSelection::OnLeftMouseReleased()
{
	Vector2f mousePos = GetMousePosition();

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
	
	m_hoverNode = GetPickedNode();

	if (m_state == State::CREATING_SELECTION_BOX)
	{
		Vector2f mins(
			Math::Min(m_selectionBoxStart.x, mousePos.x),
			Math::Min(m_selectionBoxStart.y, mousePos.y));
		Vector2f maxs(
			Math::Max(m_selectionBoxStart.x, mousePos.x),
			Math::Max(m_selectionBoxStart.y, mousePos.y));
		m_selectionBox = Rect2f(mins, maxs - mins);
	}
	else if (m_state == State::MOVING_SELECTION)
	{
		Vector2f moveAmount = mousePos - m_preMoveCursorPosition;
		for (NodeGroup* group : m_selection.GetNodeGroups())
		{
			PreMoveInfo info = m_preMoveInfo[group];
			group->SetPosition(info.position + moveAmount);
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
	if (IsControlDown() && m_keyboard->IsKeyPressed(Keys::d))
		Deselect();
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
			Vector2f center = node->GetCenter();
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
	// Get the node group the cursor is currently hovering over
	for (NodeGroup* group : m_network->GetNodeGroups())
	{
		// Check if any node in the group is hovered over
		for (int index = 0; index < group->GetNumNodes(); index++)
		{
			Node* node = group->GetNode(index);
			float radius = node->GetWidth() * 0.5f;
			Vector2f center = node->GetCenter();
			if (m_selectionBox.Contains(center))
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

