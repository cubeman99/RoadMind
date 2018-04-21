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
	const RoadMetrics* GetMetrics() const;
	NodeGroup* GetTwin() const;
	Node* GetLeftNode() const;
	Node* GetRightNode() const;
	Node* GetNode(int index);
	int GetNumNodes() const;
	float GetWidth() const;
	Vector2f GetCenterPosition() const;

	// Setters

	void SetPosition(const Vector2f& position);
	void SetDirection(const Vector2f& direction);

	// Geometry

	void UpdateGeometry();

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

	NodeGroup* m_twin;
	NodeGroupTie* m_tie;

	// Road rules
	bool m_allowPassing;

	const RoadMetrics* m_metrics;
};


#endif // _NODE_GROUP_H_