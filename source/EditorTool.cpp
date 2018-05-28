#include "EditorTool.h"

EditorTool::EditorTool()
{
}

bool EditorTool::IsShiftDown() const
{
	return (m_keyboard->IsKeyDown(Keys::left_shift) ||
		m_keyboard->IsKeyDown(Keys::right_shift));
}

bool EditorTool::IsControlDown() const
{
	return (m_keyboard->IsKeyDown(Keys::left_control) ||
		m_keyboard->IsKeyDown(Keys::right_control));
}

bool EditorTool::IsAltDown() const
{
	return (m_keyboard->IsKeyDown(Keys::left_alt) ||
		m_keyboard->IsKeyDown(Keys::right_alt));
}

Vector2f EditorTool::GetMousePosition() const
{
	return m_mousePosition;
}

Vector2f EditorTool::GetMousePositionInWindow() const
{
	return m_mousePositionInWindow;
}
