#ifndef _ROAD_NETWORK_H_
#define _ROAD_NETWORK_H_

#include "NodeGroup.h"
#include "Road.h"
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
	std::set<RoadSurface*>& GetRoadSurfaces();
	const RoadMetrics& GetMetrics() const;

	// Topology Modification
	NodeGroup* CreateNodeGroup(const Vector2f& position,
		const Vector2f& direction = Vector2f::UNITX, int laneCount = 1);
	RoadSurface* ConnectNodeGroups(NodeGroup* from, NodeGroup* to);
	RoadSurface* ConnectNodeSubGroups(
		NodeGroup* from, int fromIndex, int fromCount,
		NodeGroup* to, int toIndex, int toCount);
	NodeGroupTie* TieNodeGroups(NodeGroup* a, NodeGroup* b);
	void UntieNodeGroup(NodeGroup* nodeGroup);
	void DeleteNodeGroup(NodeGroup* nodeGroup);
	void DeleteNodeGroupConnection(RoadSurface* connection);
	void ClearNodes();

	Node* CreateNode();
	Node* CreateNode(const Vector2f& position, const Vector2f& direction, float width);
	void DeleteNode(Node* node);
	Connection* Connect(Node* from, Node* to);
	void Disconnect(Node* from, Node* to);
	void SewNode(Node* node, Node* left);
	void SewOpposite(Node* node, Node* left);

	// Geometry
	void UpdateNodeGeometry();

private:
	RoadMetrics m_metrics;
	std::set<NodeGroupTie*> m_nodeGroupTies;
	std::set<NodeGroup*> m_nodeGroups;
	std::set<RoadSurface*> m_roadSurfaces;
	std::set<Node*> m_nodes;
	std::set<Connection*> m_connections;
	unsigned int m_nodeIdCounter;
};


#endif // _ROAD_NETWORK_H_