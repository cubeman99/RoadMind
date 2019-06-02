#pragma once

#include "Driver.h"


class DrivingSystem
{
public:
	DrivingSystem(RoadNetwork* network);
	~DrivingSystem();


	inline Array<Driver*>& GetDrivers()
	{
		return m_drivers;
	}

	void Clear();
	void SpawnDriver();
	void Update(float dt);

private:
	RoadNetwork* m_network;
	Array<Driver*> m_drivers;
};
