#ifndef _ROAD_NETWORK_H_
#define _ROAD_NETWORK_H_

#include "NodeGroup.h"
#include "NodeGroupTie.h"
#include "NodeGroupConnection.h"
#include "Connection.h"
#include "RoadIntersection.h"


class RoadNetwork
{
public:
	// Constructors
	RoadNetwork();
	~RoadNetwork();

	// Getters
	Set<NodeGroup*>& GetNodeGroups();
	Set<NodeGroupConnection*>& GetNodeGroupConnections();
	Set<NodeGroupTie*>& GetNodeGroupTies();
	Set<RoadIntersection*>& GetIntersections();
	const RoadMetrics& GetMetrics() const;

	// Topology Modification

	bool GrowNodeGroup(NodeSubGroup& subGroup);

	NodeGroup* CreateNodeGroup(const Vector2f& position,
		const Vector2f& direction = Vector2f::UNITX, int laneCount = 1);
	RoadIntersection* CreateIntersection(const Set<NodeGroup*>& nodeGroups);
	Node* AddNodeToGroup(NodeGroup* group);
	NodeGroupConnection* ConnectNodeGroups(NodeGroup* from, NodeGroup* to);
	NodeGroupConnection* ConnectNodeSubGroups(
		NodeGroup* from, int fromIndex, int fromCount,
		NodeGroup* to, int toIndex, int toCount);
	NodeGroupConnection* ConnectNodeSubGroups(
		const NodeSubGroup& from, const NodeSubGroup& to);
	NodeGroupTie* TieNodeGroups(NodeGroup* a, NodeGroup* b);
	void AddNodesToLeftOfGroup(NodeGroup* group, int count = 1);
	void AddNodesToGroup(NodeGroup* group, int count = 1);
	void RemoveNodeFromGroup(NodeGroup* group, int count = 1);
	void UntieNodeGroup(NodeGroup* nodeGroup);
	void DeleteNodeGroup(NodeGroup* nodeGroup);
	void DeleteNodeGroupConnection(NodeGroupConnection* connection);
	void ClearNodes();

	Node* CreateNode();

	// Geometry
	void UpdateNodeGeometry();


private:
	RoadMetrics m_metrics;
	Set<NodeGroupTie*> m_nodeGroupTies;
	Set<NodeGroup*> m_nodeGroups;
	Set<NodeGroupConnection*> m_nodeGroupConnections;
	Set<RoadIntersection*> m_intersections;
	unsigned int m_nodeIdCounter;
};


#endif // _ROAD_NETWORK_H_