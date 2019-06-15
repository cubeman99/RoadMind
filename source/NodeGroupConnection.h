#ifndef _NODE_GROUP_CONNECTION_H_
#define _NODE_GROUP_CONNECTION_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include <cmgGraphics/cmg_graphics.h>
#include "CommonTypes.h"
#include "NodeGroup.h"
#include "Biarc3.h"
#include "RoadCurves.h"
#include "RoadSurface.h"


//-----------------------------------------------------------------------------
// Class:   NodeGroupConnection
// Purpose: Represents a one-way connection between two node (sub) groups.
//-----------------------------------------------------------------------------
class NodeGroupConnection : public RoadSurface
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
	const RoadCurveLine& GetVisualEdgeLine(LaneSide side) const;
	const RoadCurveLine& GetLeftVisualEdgeLine() const;
	const RoadCurveLine& GetRightVisualEdgeLine() const;
	int GetDividerLineCount() const;
	const RoadCurveLine& GetVisualDividerLine(int index) const;
	const RoadCurveLine& GetLeftVisualShoulderLine() const;
	const RoadCurveLine& GetRightVisualShoulderLine() const;
	NodeGroupConnection* GetTwin();
	NodeSubGroup& GetSubGroup(IOType type);
	NodeSubGroup& GetInput();
	NodeSubGroup& GetOutput();
	const Array<RoadCurveLine>& GetSeams(IOType type, LaneSide side) const;
	Array<RoadCurveLine>& GetSeams(IOType type, LaneSide side);
	RoadCurveLine GetDrivingLine(int fromLaneIndex, int toLaneIndex);
	RoadCurveLine GetDrivingLine(int laneIndex);
	void GetLaneOutputRange(int fromLaneIndex, int& outToLaneIndex, int& outToLaneCount);
	bool IsGhost() const;
	float GetLinearSlope() const;
	Mesh* GetMesh();

	// Setters

	void SetInput(const NodeSubGroup& input);
	void SetOutput(const NodeSubGroup& output);
	void SetGhost(bool ghost);
	void CycleLaneSplit();

	// Geometry

	virtual void UpdateGeometry() override;
	void CreateMesh();

public:
	int m_id;
	const RoadMetrics* m_metrics;

private:
	void SetSeam(IOType end, LaneSide side, const RoadCurveLine& seam);
	void AddSeam(IOType end, LaneSide side, const RoadCurveLine& seam);
	void ConstrainLaneSplit();


public:
	// Properties
	bool m_isGhost;
	Array<int> m_laneSplit;
	NodeSubGroup m_groups[2]; // Input and Output groups

	// Generated geometry
	RoadCurveLine m_leftLaneEdge;
	RoadCurveLine* m_visualEdgeLines[2];
	Array<RoadCurveLine> m_visualDividerLines;
	RoadCurveLine m_visualShoulderLines[2];
	Array<RoadCurveLine> m_seams[2][2];

	// Meshes
	Mesh* m_mesh;
};


#endif // _NODE_GROUP_CONNECTION_H_