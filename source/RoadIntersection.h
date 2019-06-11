#pragma once

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "NodeGroup.h"
#include "RoadSurface.h"
#include "TrafficLight.h"
#include <set>

class Connection;
class NodeGroup;
class NodeGroupConnection;
class RoadIntersectionPoint;


class RoadIntersectionEdge
{
	friend class RoadIntersection;
	friend class RoadNetwork;

public:
	RoadIntersectionEdge()
	{
	}
	RoadIntersectionEdge(RoadIntersectionPoint* leftPoint,
		RoadIntersectionPoint* rightPoint)
	{
		m_points[0] = leftPoint;
		m_points[1] = rightPoint;
	}
	RoadIntersectionEdge(RoadIntersectionPoint* leftPoint,
		RoadIntersectionPoint* rightPoint, const BiarcPair& shoulderEdge,
		const BiarcPair& laneEdge)
		: m_shoulderEdge(shoulderEdge)
		, m_laneEdge(laneEdge)
	{
		m_points[0] = leftPoint;
		m_points[1] = rightPoint;
	}

	inline RoadIntersectionPoint* GetPoint(LaneSide side)
	{
		return m_points[(int) side];
	}
	inline const BiarcPair& GetShoulderEdge() const
	{
		return m_shoulderEdge;
	}
	inline const BiarcPair& GetLaneEdge() const
	{
		return m_laneEdge;
	}

private:
public:
	RoadIntersectionPoint* m_points[2];
	BiarcPair m_shoulderEdge;
	BiarcPair m_laneEdge;
	Biarc* m_line;
	Biarc* m_arc;
	Biarc m_halfArcs[2];
};


class RoadIntersectionPoint
{
	friend class RoadIntersection;
	friend class RoadNetwork;

public:
	RoadIntersectionPoint()
	{
	}

	RoadIntersectionPoint(NodeGroup* group, IOType type)
		: m_nodeGroup(group)
		, m_ioType(type)
	{
	}

	inline NodeGroup* GetNodeGroup()
	{
		return m_nodeGroup;
	}
	inline IOType GetIOType() const
	{
		return m_ioType;
	}

	inline NodeGroupConnection* GetConnection(LaneSide side)
	{
		if (m_ioType == IOType::INPUT)
		{
			if (side == LaneSide::RIGHT)
				return m_nodeGroup->GetInputs().back();
			else
				return m_nodeGroup->GetInputs().front();
		}
		else
		{
			if (side == LaneSide::RIGHT)
				return m_nodeGroup->GetOutputs().back();
			else
				return m_nodeGroup->GetOutputs().front();
		}
	}

private:
	NodeGroup* m_nodeGroup;
	IOType m_ioType; // input moves toward the intersection, output moves away from it

	// TODO: road rules
};


class RoadIntersection : public RoadSurface
{
public:
	friend class RoadNetwork;

public:
	// Constructors

	RoadIntersection();
	~RoadIntersection();

	// Getters
	Vector2f GetCenterPosition() const;
	Array<RoadIntersectionPoint*>& GetPoints();
	Array<RoadIntersectionEdge*>& GetEdges();
	const TrafficLightProgram* GetTrafficLightProgram() const;

	// Setters
	TrafficLightProgram* CreateTrafficLightProgram();

	// Geometry
	void Update(Seconds dt);
	virtual void UpdateGeometry() override;

private:
	void Construct(const Set<NodeGroup*>& nodeGroups);
	RoadIntersectionPoint* AddPoint(NodeGroup* group, IOType type);

	int m_id;
	Vector2f m_centerPosition;
	TrafficLightProgram* m_trafficLightProgram;

	// Sorted in clockwise order
	Array<RoadIntersectionPoint*> m_points;
	Array<RoadIntersectionEdge*> m_edges;
};

