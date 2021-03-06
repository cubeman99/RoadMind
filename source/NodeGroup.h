#ifndef _NODE_GROUP_H_
#define _NODE_GROUP_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Node.h"

class NodeGroupTie;
class NodeGroup;
class RoadIntersection;


//-----------------------------------------------------------------------------
// Struct:  NodeSubGroup
// Purpose: Identifies a subset of a node group.
//-----------------------------------------------------------------------------
struct NodeSubGroup
{
public:
	NodeGroup* group;
	int index;
	int count;

public:
	// Constructors

	NodeSubGroup();
	NodeSubGroup(NodeGroup* group, int index, int count);

	// Getters

	float GetWidth() const;
	Vector3f GetLeftPosition() const;
	Vector3f GetCenterPosition() const;
	Node* GetNode(int index);
	bool ContainsNode(Node* node);

	// Static methods

	static int GetOverlap(const NodeSubGroup& a, const NodeSubGroup& b);
};


//-----------------------------------------------------------------------------
// Class:   NodeGroup
// Purpose: Represents a group of adjacent lane nodes with a shared direction.
//-----------------------------------------------------------------------------
class NodeGroup : public IPosition, public ECSComponent<NodeGroup>
{
	friend class RoadNetwork;
	friend class RoadIntersection;
	friend class NodeGroupTie;
	friend class NodeGroupConnection;

public:
	// Constructors

	NodeGroup();
	~NodeGroup();

	// Getters

	const Vector3f& GetPosition() const;
	const Vector2f& GetDirection() const;
	Vector2f GetLeftDirection() const;
	Vector2f GetRightDirection() const;
	const RoadMetrics* GetMetrics() const;
	NodeGroup* GetTwin() const;
	RoadIntersection* GetIntersection(IOType type = IOType::OUTPUT) const;
	Node* GetLeftNode() const;
	Node* GetRightNode() const;
	Node* GetNode(int index);
	NodeGroupTie* GetTie();
	int GetNumNodes() const;
	Meters GetWidth() const;
	Meters GetShoulderWidth(LaneSide side) const;
	Meters GetRightShoulderWidth() const;
	Meters GetLeftShoulderWidth() const;
	Vector3f GetCenterPosition() const;
	Vector3f GetRightPosition() const;
	Array<NodeGroupConnection*>& GetInputs();
	Array<NodeGroupConnection*>& GetOutputs();
	bool IsTied() const;
	RightOfWay GetRightOfWay() const;
	float GetSlope() const;

	// Setters

	void SetPosition(const Vector3f& position);
	void SetAltitude(float z);
	void SetDirection(const Vector2f& direction);
	void SetDirectionFromCenter(const Vector2f& direction);
	void SetRightOfWay(RightOfWay rightOfWay);

	// Geometry

	bool IntersectConnections(NodeGroupConnection* a, NodeGroupConnection* b, IOType end);
	bool IntersectEdgeLines(NodeGroupConnection* a, NodeGroupConnection* b, IOType end);
	void UpdateGeometry();
	void UpdateIntersectionGeometry();

private:

	void InsertInput(NodeGroupConnection* input);
	void InsertOutput(NodeGroupConnection* output);
	void InsertConnection(NodeGroupConnection* connection, int direction);
	void RemoveInput(NodeGroupConnection* input);
	void RemoveOutput(NodeGroupConnection* output);
	void RemoveConnection(NodeGroupConnection* connection, int direction);
	void UpdateConnectionSorting(bool search = true);

private:
	int m_id;

	// Position
	Vector3f m_position;
	Vector2f m_direction;
	float m_slope;

	// Connections
	Array<Node*> m_nodes;
	NodeGroup* m_twin;
	NodeGroupTie* m_tie;
	RoadIntersection* m_intersection;
	RoadIntersection* m_inputIntersection;
	Array<NodeGroupConnection*> m_connections[2];

	// Road rules
	bool m_allowPassing;
	RightOfWay m_rightOfWay;

	Meters m_leftShoulderWidth;
	Meters m_rightShoulderWidth;
	const RoadMetrics* m_metrics;
};


#endif // _NODE_GROUP_H_