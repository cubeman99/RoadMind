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

public:
	// Constructors

	NodeGroupConnection();
	~NodeGroupConnection();

	// Getters

	const RoadMetrics* GetMetrics() const;
	BiarcPair GetLeftEdgeLine() const;
	BiarcPair GetRightEdgeLine() const;
	NodeGroupConnection* GetTwin();
	const NodeSubGroup& GetInput();
	const NodeSubGroup& GetOutput();

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

public:
	std::vector<BiarcPair> m_dividerLines;
};



#endif // _ROAD_H_