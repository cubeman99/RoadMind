#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Biarc.h"

class Node;


class Connection
{
	friend class RoadNetwork;

public:
	Connection(Node* from, Node* to);
	~Connection();

	Node* GetInput();
	Node* GetOutput();
	
	//float GetDistance() const;

	Connection* GetLeftConnection() const;
	Connection* GetRightConnection() const;
	bool IsLeftMostLane() const;
	bool IsRightMostLane() const;
	
	LaneDivider GetLaneDivider(LaneSide side) const;

	//Vector2f GetPoint(Meters distance, LaneSide side, Meters offset = 0.0f);

private:
	Node* m_input;
	Node* m_output;
};


#endif // _CONNECTION_H_