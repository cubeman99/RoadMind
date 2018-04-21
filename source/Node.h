#ifndef _NODE_H_
#define _NODE_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Biarc.h"
#include <set>

class Connection;
class RoadNetwork;
class NodeGroup;


class Node
{
	friend class Connection;
	friend class RoadNetwork;
	friend class NodeGroup;
	friend class RoadSurface;

public:
	// Constructors

	Node();
	Node(const Vector2f& position, const Vector2f& direction,
		const Vector2f& leftDirection, const Vector2f& rightDirection,
		float width);

	// Getters

	NodeGroup* GetNodeGroup();
	const RoadMetrics* GetMetrics() const;
	float GetWidth() const;
	unsigned int GetNodeId() const;
	Vector2f GetEndNormal() const;
	Vector2f GetEndTangent() const;
	Vector2f GetLeftEdge() const;
	Vector2f GetRightEdge() const;
	Vector2f GetLeftEdgeTangent() const;
	Vector2f GetRightEdgeTangent() const;
	Vector2f GetCenter() const;
	Node* GetLeftNode() const;
	Node* GetRightNode() const;
	int GetNumInputs() const;
	int GetNumOutputs() const;
	int GetNumConnections(InputOutput type) const;
	std::set<Connection*>& GetConnections(InputOutput type);
	std::set<Connection*>& GetInputs();
	std::set<Connection*>& GetOutputs();
	bool HasInput(Node* node) const;
	bool HasOutput(Node* node) const;

	Connection* GetInputConnection(Node* node) const;
	Connection* GetOutputConnection(Node* node) const;

	LaneDivider GetLeftLaneDivider() const;
	LaneDivider GetRightLaneDivider() const;
	bool IsLeftMostLane() const;
	bool IsRightMostLane() const;

	// Setters

	void SetPrevNode(Node* node);
	void SetNextNode(Node* node);
	void SetLeftNode(Node* node);
	void SetRightNode(Node* node);
	void SetWidth(float width);
	void SetLeftPosition(const Vector2f& position);
	void SetCenterPosition(const Vector2f& center);
	void SetEndNormal(const Vector2f& normal);
	void SetLeftEdgeTangent(const Vector2f& tangent);
	void SetRightEdgeTangent(const Vector2f& tangent);

private:
	void UpdateGeometry();
	
	unsigned int m_nodeId;
	Meters m_width;
	Vector2f m_position;
	Vector2f m_endNormal;
	Vector2f m_rightTangent;
	Vector2f m_leftTangent;

	LaneDivider m_leftDivider;

	union
	{
		struct
		{
			Node* m_leftNode;
			Node* m_rightNode;
		};
		Node* m_sideNodes[2];
	};

	std::set<Connection*> m_outputs;
	std::set<Connection*> m_inputs;

	void* m_signal; // Stop-light, stop-sign, yield-sign
	void* m_laneMarking;

	const RoadMetrics* m_metrics;
	NodeGroup* m_nodeGroup;

	Array<Vector2f> m_vertices[3];
};


#endif // _NODE_H_