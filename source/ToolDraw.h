#ifndef _TOOL_DRAW_H_
#define _TOOL_DRAW_H_

#include "EditorTool.h"
#include "RoadNetwork.h"

enum class DragState
{
	NONE,
	POSITION,
	DIRECTION,
};


class ToolDraw : public EditorTool
{
public:
	struct DragInfo
	{
		DragState state;
		NodeGroup* inputGroup;
		NodeGroup* nodeGroup;
		NodeGroupConnection* connection;
		Vector2f position;
	};

	struct HoverInfo
	{
		NodeSubGroup subGroup;
		bool reverse;
		Vector2f center;
		int nodeIndex;
		float nodePartialIndex;
		bool isValidSubGroup;
	};
	
	struct SnapInfo
	{
		bool reverse;
		//Vector2f center;
		NodeSubGroup subGroup;
	};

public:
	ToolDraw();

	void OnBegin() override;
	void OnEnd() override;
	void Update(float dt) override;

	void OnLeftMousePressed() override;
	void OnRightMousePressed() override;
	void OnLeftMouseReleased() override;
	void OnRightMouseReleased() override;

	void CancelDragging();
	void StopDragging();

	int GetLaneCount() const;

	inline const HoverInfo& GetHoverInfo() const
	{
		return m_hoverInfo;
	}

private:
	NodeGroup* GetPickedNodeGroup();
	void UpdateHoverInfo();
	void UdpateDragging(float dt);
	int GetBestCurve(const BiarcPair& a, const BiarcPair& b);

	int m_laneCount;
	DragInfo m_dragInfo;
	HoverInfo m_hoverInfo;
	SnapInfo m_snapInfo;
	Vector2f m_mousePositionInWindowPrev;
};


#endif // _TOOL_DRAW_H_