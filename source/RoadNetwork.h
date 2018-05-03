#ifndef _ROAD_NETWORK_H_
#define _ROAD_NETWORK_H_

#include "NodeGroup.h"
#include "NodeGroupTie.h"
#include "NodeGroupConnection.h"
#include "Connection.h"
#include <set>


class RoadNetwork
{
public:
	// Constructors
	RoadNetwork();
	~RoadNetwork();

	// Getters
	std::set<Node*>& GetNodes();
	std::set<Connection*>& GetConnections();
	std::set<NodeGroup*>& GetNodeGroups();
	std::set<NodeGroupTie*>& GetNodeGroupTies();
	std::set<NodeGroupConnection*>& GetNodeGroupConnections();
	const RoadMetrics& GetMetrics() const;

	// Topology Modification
	NodeGroup* CreateNodeGroup(const Vector2f& position,
		const Vector2f& direction = Vector2f::UNITX, int laneCount = 1);
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
	Node* CreateNode(const Vector2f& position, const Vector2f& direction, float width);
	void DeleteNode(Node* node);
	Connection* Connect(Node* from, Node* to);
	void Disconnect(Node* from, Node* to);
	void DeleteConnection(Connection* connection);

	// Geometry
	void UpdateNodeGeometry();

private:
	RoadMetrics m_metrics;
	std::set<NodeGroupTie*> m_nodeGroupTies;
	std::set<NodeGroup*> m_nodeGroups;
	std::set<NodeGroupConnection*> m_nodeGroupConnections;
	std::set<Node*> m_nodes;
	std::set<Connection*> m_connections;
	unsigned int m_nodeIdCounter;
};


#endif // _ROAD_NETWORK_H_