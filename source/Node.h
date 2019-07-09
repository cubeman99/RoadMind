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

	// Getters

	int GetIndex() const;
	NodeGroup* GetNodeGroup();
	const RoadMetrics* GetMetrics() const;
	float GetWidth() const;
	Vector2f GetDirection() const;
	Vector2f GetEndTangent() const;
	Vector3f GetPosition() const;
	Vector3f GetLeftEdge() const;
	Vector3f GetRightEdge() const;
	Vector2f GetLeftEdgeTangent() const;
	Vector2f GetRightEdgeTangent() const;
	Vector3f GetCenter() const;
	Node* GetLeftNode() const;
	Node* GetRightNode() const;
	bool IsLeftMostLane() const;
	bool IsRightMostLane() const;
	bool HasStopSign() const;
	TrafficLightSignal GetSignal() const;

	// Setters

	void SetWidth(float width);
	void SetLeftPosition(const Vector3f& position);
	void SetCenterPosition(const Vector3f& center);
	void SetEndNormal(const Vector2f& normal);
	
private:
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

private:
	Meters m_width;
	Vector3f m_position;
	Vector2f m_direction;
	LaneDivider m_leftDivider;

	int m_index;

	void* m_laneMarking;
	bool m_hasStopSign;

	const RoadMetrics* m_metrics;
	NodeGroup* m_nodeGroup;

	Array<Vector2f> m_vertices[3];

	// UNUSED
	Set<Connection*> m_outputs;
	Set<Connection*> m_inputs;
};


#endif // _NODE_H_