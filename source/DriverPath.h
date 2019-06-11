#pragma once

#include "NodeGroupConnection.h"
#include "RoadIntersection.h"

class DrivingSystem;


class DriverPathNode
{
public:
	DriverPathNode()
		: m_connection(nullptr)
	{}
	DriverPathNode(NodeGroupConnection* connection,
		int startLaneIndex, int endLaneIndex, int laneShift) 
		: m_connection(connection)
		, m_surface(connection)
		, m_laneIndexStart(startLaneIndex)
		, m_laneIndexEnd(endLaneIndex)
		, m_laneShift(laneShift)
	{
		m_nodeStart = m_connection->GetInput().GetNode(m_laneIndexStart);
		m_nodeEnd = m_connection->GetOutput().GetNode(m_laneIndexEnd);
		m_drivingLine = m_connection->GetDrivingLine(
			startLaneIndex, endLaneIndex);
	}

	DriverPathNode(RoadIntersection* intersection,
		Node* startNode, Node* endNode, int laneShift=0)
		: m_connection(nullptr)
		, m_surface(intersection)
		, m_laneIndexStart(startNode->GetIndex())
		, m_laneIndexEnd(endNode->GetIndex())
		, m_laneShift(laneShift)
		, m_nodeStart(startNode)
		, m_nodeEnd(endNode)
	{
		m_drivingLine = BiarcPair::Interpolate(
			startNode->GetCenter().xy, startNode->GetDirection(),
			endNode->GetCenter().xy, endNode->GetDirection());
	}

	inline Node* GetStartNode() const {
		return m_nodeStart;
	}
	inline Node* GetEndNode() const {
		return m_nodeEnd;
	}
	inline RoadSurface* GetSurface() const {
		return m_surface;
	}
	inline const BiarcPair& GetDrivingLine() const {
		return m_drivingLine;
	}
	inline Meters GetDistance() const {
		return m_drivingLine.Length();
	}
	inline int GetLaneShift() const {
		return m_laneShift;
	}

private:
	NodeGroupConnection* m_connection;
	RoadSurface* m_surface;
	Node* m_nodeStart;
	Node* m_nodeEnd;
	int m_laneIndexStart;
	int m_laneIndexEnd; // Relative to connection left lane
	int m_laneShift;
	BiarcPair m_drivingLine;
};

