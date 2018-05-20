#ifndef _ROAD_H_
#define _ROAD_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "NodeGroup.h"


//-----------------------------------------------------------------------------
// Class:   NodeGroupConnection
// Purpose: Represents a one-way connection between two node (sub) groups.
//-----------------------------------------------------------------------------
class NodeGroupConnection
{
	friend class RoadNetwork;
	friend class NodeGroup;
	friend class RoadIntersection;

public:
	// Constructors

	NodeGroupConnection();
	~NodeGroupConnection();

	// Getters

	const RoadMetrics* GetMetrics() const;
	BiarcPair GetLeftEdgeLine() const;
	BiarcPair GetRightEdgeLine() const;
	BiarcPair GetLeftVisualEdgeLine() const;
	BiarcPair GetRightVisualEdgeLine() const;
	BiarcPair GetLeftVisualShoulderLine() const;
	BiarcPair GetRightVisualShoulderLine() const;
	NodeGroupConnection* GetTwin();
	const NodeSubGroup& GetInput();
	const NodeSubGroup& GetOutput();
	const Array<BiarcPair>& GetSeams(IOType type, LaneSide side) const;
	Array<BiarcPair>& GetSeams(IOType type, LaneSide side);

	// Geometry

	void UpdateGeometry();


public:
	union
	{
		struct
		{
			NodeSubGroup m_input;
			NodeSubGroup m_output;
		};
		struct
		{
			NodeSubGroup m_groups[2];
		};
	};

	NodeGroupConnection* m_twin;

	const RoadMetrics* m_metrics;

private:
	void SetSeam(IOType end, LaneSide side, const BiarcPair& seam);
	void AddSeam(IOType end, LaneSide side, const BiarcPair& seam);


public:
	BiarcPair m_edgeLines[2];
	std::vector<BiarcPair> m_dividerLines;
	BiarcPair m_visualEdgeLines[2];
	BiarcPair m_visualShoulderLines[2];
	Array<BiarcPair> m_seams[2][2];
	Vector2f m_laneIntersectionPoint;
	Vector2f m_edgeIntersectionPoint;
};



#endif // _ROAD_H_