#ifndef _ROAD_H_
#define _ROAD_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "NodeGroup.h"


//-----------------------------------------------------------------------------
// Class:   RoadSurface
// Purpose: Represents a one-way connection between two node (sub) groups.
//-----------------------------------------------------------------------------
class RoadSurface
{
	friend class RoadNetwork;

public:
	// Constructors
	RoadSurface();
	~RoadSurface();

	// Getters
	const RoadMetrics* GetMetrics() const;
	BiarcPair GetLeftEdgeLine() const;
	BiarcPair GetRightEdgeLine() const;
	RoadSurface* GetTwin();
	NodeGroup* GetInput();
	NodeGroup* GetOutput();

	// Setters
	void UpdateGeometry();
	
private:
	const RoadMetrics* m_metrics;

	NodeGroup* m_groups[2];
	int m_indexes[2];
	int m_counts[2];
	RoadSurface* m_twin;

public:
	std::vector<BiarcPair> m_dividerLines;
};


#endif // _ROAD_H_