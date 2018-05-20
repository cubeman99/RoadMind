#ifndef _NODE_GROUP_H_
#define _NODE_GROUP_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Node.h"

class NodeGroupTie;
class NodeGroup;


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

	// Constructors

	NodeSubGroup()
		: group(nullptr)
		, index(0)
		, count(0)
	{
	}

	NodeSubGroup(NodeGroup* group, int index, int count)
		: group(group)
		, index(index)
		, count(count)
	{
	}

	// Getters

	float GetWidth() const;
	Vector2f GetLeftPosition() const;
	Vector2f GetCenterPosition() const;

public:

	// Static methods

	static int GetOverlap(const NodeSubGroup& a, const NodeSubGroup& b)
	{
		return Math::Min(
			a.index + a.count - b.index, 
			b.index + b.count - a.index);
	}
};


//-----------------------------------------------------------------------------
// Class:   NodeGroup
// Purpose: Represents a group of adjacent lane nodes with a shared direction.
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
	Vector2f GetLeftDirection() const;
	Vector2f GetRightDirection() const;
	const RoadMetrics* GetMetrics() const;
	NodeGroup* GetTwin() const;
	Node* GetLeftNode() const;
	Node* GetRightNode() const;
	Node* GetNode(int index);
	NodeGroupTie* GetTie();
	int GetNumNodes() const;
	Meters GetWidth() const;
	Meters GetShoulderWidth(LaneSide side) const;
	Meters GetRightShoulderWidth() const;
	Meters GetLeftShoulderWidth() const;
	Vector2f GetCenterPosition() const;
	Vector2f GetRightPosition() const;
	Array<NodeGroupConnection*>& GetInputs();
	Array<NodeGroupConnection*>& GetOutputs();

	// Setters

	void SetPosition(const Vector2f& position);
	void SetDirection(const Vector2f& direction);
	void SetDirectionFromCenter(const Vector2f& direction);

	// Geometry

	bool IntersectConnections(NodeGroupConnection* a, NodeGroupConnection* b, IOType end);
	void UpdateGeometry();
	void UpdateIntersectionGeometry();

private:

	void InsertInput(NodeGroupConnection* input);
	void InsertOutput(NodeGroupConnection* output);
	void InsertConnection(NodeGroupConnection* connection, int direction);
	void RemoveInput(NodeGroupConnection* input);
	void RemoveOutput(NodeGroupConnection* output);
	void RemoveConnection(NodeGroupConnection* connection, int direction);

	Vector2f m_position;
	Vector2f m_direction;

	Array<Node*> m_nodes;

	union
	{
		struct
		{
			Array<NodeGroupConnection*> m_inputs;
			Array<NodeGroupConnection*> m_outputs;
		};
		struct
		{
			Array<NodeGroupConnection*> m_connections[2];
		};
	};

	Meters m_leftShoulderWidth;
	Meters m_rightShoulderWidth;

	NodeGroup* m_twin;
	NodeGroupTie* m_tie;

	// Road rules
	bool m_allowPassing;

	const RoadMetrics* m_metrics;
};


#endif // _NODE_GROUP_H_