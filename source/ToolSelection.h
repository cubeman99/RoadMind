#ifndef _TOOL_SELECTION_H_
#define _TOOL_SELECTION_H_

#include "EditorTool.h"
#include "NodeGroup.h"
#include <map>


class NodeGroupSelection
{
public:
	NodeGroupSelection()
	{
	}

	inline Set<NodeGroup*>& GetNodeGroups()
	{
		return m_nodeGroups;
	}

	inline void Clear()
	{
		m_nodeGroups.clear();
	}

	inline void Add(NodeGroup* group)
	{
		m_nodeGroups.insert(group);
	}

	inline void Remove(NodeGroup* group)
	{
		m_nodeGroups.erase(group);
	}

	inline bool IsEmpty() const
	{
		return m_nodeGroups.empty();
	}

	inline int GetNumGroups() const
	{
		return (int) m_nodeGroups.size();
	}

	inline bool Contains(Node* node)
	{
		return Contains(node->GetNodeGroup());
	}

	inline bool Contains(NodeGroup* group)
	{
		return (m_nodeGroups.find(group) != m_nodeGroups.end());
	}

private:
	Set<NodeGroup*> m_nodeGroups;
};


enum class SelectMode
{
	ADD,
	REMOVE,
};


class ToolSelection : public EditorTool
{
public:
	ToolSelection();
	
	void OnBegin() override;
	void OnEnd() override;
	void OnLeftMousePressed() override;
	void OnRightMousePressed() override;
	void OnLeftMouseReleased() override;
	void OnRightMouseReleased() override;
	void Update(float dt) override;
	
	bool IsCreatingSelection() const;
	Rect2f GetSelectionBox() const;
	NodeGroupSelection& GetSelection();
	void Deselect();
	void DeleteSelection();
	void Select(const Rect2f& box, SelectMode mode = SelectMode::ADD);

	void CancelMovement();
	void StopMovement();

	Node* GetPickedNode();


private:
	enum class State
	{
		NONE,
		CREATING_SELECTION_BOX,
		MOVING_SELECTION,
	};

	Vector2f m_selectionBoxStart;
	Rect2f m_selectionBox;
	State m_state;
	NodeGroupSelection m_selection;
	Node* m_hoverNode;

	Vector2f m_preMoveCursorPosition;
	struct PreMoveInfo
	{
		Vector3f position;
		Vector2f direction;
	};

	std::map<NodeGroup*, PreMoveInfo> m_preMoveInfo;
};


#endif // _TOOL_SELECTION_H_