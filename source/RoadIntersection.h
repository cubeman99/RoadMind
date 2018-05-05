#ifndef _ROAD_INTERSECTION_H_
#define _ROAD_INTERSECTION_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Node.h"
#include <set>

class Connection;
class NodeGroup;
class NodeGroupConnection;


class RoadIntersection
{
public:
	// Constructors

	RoadIntersection();
	~RoadIntersection();

	// Getters
	
	Array<NodeGroup*>& GetNodeGroups();

	// Setters
		
	// Geometry

	void UpdateGeometry();

private:
	// Node groups sorted in clockwise order
	Array<NodeGroup*> m_nodeGroups;
};


#endif // _ROAD_INTERSECTION_H_