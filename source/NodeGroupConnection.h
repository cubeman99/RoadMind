#ifndef _NODE_GROUP_CONNECTION_H_
#define _NODE_GROUP_CONNECTION_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "NodeGroup.h"
#include "Biarc3.h"
#include "RoadCurves.h"

class Driver;


//-----------------------------------------------------------------------------
// Class:   NodeGroupConnection
// Purpose: Represents a one-way connection between two node (sub) groups.
//-----------------------------------------------------------------------------
class NodeGroupConnection
{
	friend class RoadNetwork;
	friend class NodeGroup;
	friend class RoadIntersection;

public:
	// Constructors

	NodeGroupConnection();
	~NodeGroupConnection();

	// Getters
	
	int GetId() const;
	const RoadMetrics* GetMetrics() const;
	BiarcPair GetLeftEdgeLine() const;
	BiarcPair GetRightEdgeLine() const;
	RoadCurveLine GetLeftVisualEdgeLine() const;
	RoadCurveLine GetRightVisualEdgeLine() const;
	RoadCurveLine GetLeftVisualShoulderLine() const;
	RoadCurveLine GetRightVisualShoulderLine() const;
	NodeGroupConnection* GetTwin();
	NodeSubGroup& GetInput();
	NodeSubGroup& GetOutput();
	const Array<BiarcPair>& GetSeams(IOType type, LaneSide side) const;
	Array<BiarcPair>& GetSeams(IOType type, LaneSide side);
	Array<BiarcPair>& GetDrivingLines();
	BiarcPair GetDrivingLine(int fromLaneIndex, int toLaneIndex);
	BiarcPair GetDrivingLine(int laneIndex);
	Set<Driver*>& GetDrivers();

	// Setters

	void NodeGroupConnection::SetInput(const NodeSubGroup& input);
	void NodeGroupConnection::SetOutput(const NodeSubGroup& output);
	void AddDriver(Driver* driver);
	void RemoveDriver(Driver* driver);

	// Geometry

	void UpdateGeometry();


public:
	int m_id;
	const RoadMetrics* m_metrics;

private:
	void SetSeam(IOType end, LaneSide side, const BiarcPair& seam);
	void AddSeam(IOType end, LaneSide side, const BiarcPair& seam);


public:
	// Input and Output groups
	NodeSubGroup m_groups[2];
	BiarcPair m_edgeLines[2];
	std::vector<BiarcPair> m_dividerLines;
	std::vector<BiarcPair> m_drivingLines;
	RoadCurveLine m_visualEdgeLines[2];
	RoadCurveLine m_visualShoulderLines[2];
	Array<BiarcPair> m_seams[2][2];
	Vector2f m_laneIntersectionPoint;
	Vector2f m_edgeIntersectionPoint;
	Set<Driver*> m_drivers;
};


#endif // _NODE_GROUP_CONNECTION_H_