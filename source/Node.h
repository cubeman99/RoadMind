#ifndef _NODE_H_
#define _NODE_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Biarc.h"
#include <set>

class Connection;
class NodeGroup;
class NodeGroupConnection;


class Node
{
	friend class Connection;
	friend class RoadNetwork;
	friend class NodeGroup;
	friend class NodeGroupConnection;

public:
	// Constructors

	Node();
	Node(const Vector2f& position, const Vector2f& direction,
		const Vector2f& leftDirection, const Vector2f& rightDirection,
		float width);

	// Getters

	int GetIndex() const;
	NodeGroup* GetNodeGroup();
	const RoadMetrics* GetMetrics() const;
	float GetWidth() const;
	unsigned int GetNodeId() const;
	Vector2f GetDirection() const;
	Vector2f GetEndTangent() const;
	Vector2f GetPosition() const;
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
	void SetWidth(float width);
	void SetLeftPosition(const Vector2f& position);
	void SetCenterPosition(const Vector2f& center);
	void SetEndNormal(const Vector2f& normal);
	
	// Geometry

	void UpdateGeometry();

private:
	unsigned int m_nodeId;
	Meters m_width;
	Vector2f m_position;
	Vector2f m_direction;
	LaneDivider m_leftDivider;

	int m_index;

	Set<Connection*> m_outputs;
	Set<Connection*> m_inputs;

	void* m_signal; // Stop-light, stop-sign, yield-sign
	void* m_laneMarking;

	const RoadMetrics* m_metrics;
	NodeGroup* m_nodeGroup;

	Array<Vector2f> m_vertices[3];
};


#endif // _NODE_H_