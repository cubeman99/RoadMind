#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "NodeGroupConnection.h"


class Driver
{
public:
	Driver();
	Driver(RoadNetwork* network, Node* node);
	~Driver();

	inline Vector3f GetPosition() const
	{
		return m_position;
	}

	inline Vector2f GetDirection() const
	{
		return m_direction;
	}

	void Next();
	void Update(float dt);

private:
	Vector3f m_position;
	Vector2f m_direction;
	Vector2f m_velocity;
	Node* m_nodeCurrent;
	Node* m_nodeTarget;
	NodeGroupConnection* m_connection;
	int m_laneIndexCurrent;
	int m_laneIndexTarget; // Relative to node group lanes
	BiarcPair m_drivingLineCurrent;
	BiarcPair m_drivingLineTarget;
	RoadNetwork* m_roadNetwork;
	float m_distance;
	float m_speed;
};


#endif // _DRIVER_H_