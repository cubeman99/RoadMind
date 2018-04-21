#ifndef _NODE_GROUP_H_
#define _NODE_GROUP_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Node.h"


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
	const Vector2f& GetPosition() const;
	const Vector2f& GetDirection() const;
	Meters GetCenterWidth() const;

	// Setters
	void SetPosition(const Vector2f& position);
	void SetDirection(const Vector2f& direction);
	void SetCenterWidth(Meters centerWidth);

	// Geometry
	void UpdateGeometry();

private:
	Vector2f m_position;
	Vector2f m_direction;
	NodeGroup* m_nodeGroup;
	Meters m_centerDividerWidth;
};


//-----------------------------------------------------------------------------
// Class:   NodeGroup
// Purpose: Represents a group of adjacent lane nodes with a shared direction
//-----------------------------------------------------------------------------
class NodeGroup
{
	friend class RoadNetwork;

public:
	// Constructors
	NodeGroup();
	~NodeGroup();

	// Getters
	const Vector2f& GetPosition() const;
	const Vector2f& GetDirection() const;
	const RoadMetrics* GetMetrics() const;
	NodeGroup* GetTwin() const;
	Node* GetLeftNode() const;
	Node* GetRightNode() const;
	Node* GetNode(int index);
	int GetNumNodes() const;

	// Setters
	void SetPosition(const Vector2f& position);
	void SetDirection(const Vector2f& direction);

	// Geometry
	void UpdateGeometry();

private:
	Vector2f m_position;
	Vector2f m_direction;
	//Node* m_centerNode;
	Array<Node*> m_nodes;
	NodeGroup* m_twin;
	NodeGroupTie* m_tie;
	const RoadMetrics* m_metrics;
};


#endif // _NODE_GROUP_H_