#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "Node.h"
#include "Connection.h"


class Driver
{
public:
	Driver();
	Driver(RoadNetwork* network, Node* node);
	~Driver();

	inline Vector2f GetPosition() const
	{
		return m_position;
	}

	inline Vector2f GetDirection() const
	{
		return m_direction;
	}

	void Update(float dt);

private:
	Vector2f m_position;
	Vector2f m_direction;
	Vector2f m_velocity;
	Connection* m_lane;
	RoadNetwork* m_roadNetwork;
	float m_distance;
	float m_speed;
};


#endif // _DRIVER_H_