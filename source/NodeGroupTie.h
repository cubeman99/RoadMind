#ifndef _NODE_GROUP_TIE_H_
#define _NODE_GROUP_TIE_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "NodeGroup.h"


//-----------------------------------------------------------------------------
// Class:   NodeGroupTie
// Purpose: Ties together two node groups of opposite directions.
//-----------------------------------------------------------------------------
class NodeGroupTie
{
	friend class RoadNetwork;

public:
	// Constructors & destructors
	NodeGroupTie();
	~NodeGroupTie();
	
	// Getters
	NodeGroup* GetNodeGroupTwin() const;
	NodeGroup* GetNodeGroup() const;
	const Vector3f& GetPosition() const;
	const Vector2f& GetDirection() const;
	Meters GetCenterWidth() const;
	bool IsDivided() const;

	// Setters
	void SetPosition(const Vector3f& position);
	void SetDirection(const Vector2f& direction);
	void SetCenterWidth(Meters centerWidth);

	// Geometry
	void UpdateGeometry();

private:
	Vector3f m_position;
	Vector2f m_direction;
	NodeGroup* m_nodeGroup;
	Meters m_centerDividerWidth;
};


#endif // _NODE_GROUP_TIE_H_