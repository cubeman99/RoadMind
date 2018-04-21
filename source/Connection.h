#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "Biarc.h"

class Node;


class Connection
{
public:
	Connection(Node* from, Node* to);
	~Connection();

	Node* GetInput();
	Node* GetOutput();
	const Array<Vector2f>& GetVertices(LaneSide side) const;
	
	float GetDistance() const;

	Connection* GetLeftConnection() const;
	Connection* GetRightConnection() const;
	bool IsLeftMostLane() const;
	bool IsRightMostLane() const;
	
	LaneDivider GetLaneDivider(LaneSide side) const;

	Vector2f GetPoint(Meters distance, LaneSide side, Meters offset = 0.0f);

	void CalcVertices();

private:
	Node* m_input;
	Node* m_output;

	Biarc m_centerArc1;
	Biarc m_centerArc2;
	Biarc m_leftArc1;
	Biarc m_leftArc2;
	Biarc m_rightArc1;
	Biarc m_rightArc2;

	Array<Vector2f> m_vertices[3];
};


#endif // _CONNECTION_H_