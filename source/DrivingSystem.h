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

	float GetTrafficPercent();

	void Clear();
	void SpawnDriver();
	void DeleteDriver(Driver* driver);
	void Update(float dt);

private:
	RoadNetwork* m_network;
	Array<Driver*> m_drivers;
	float m_trafficPercent;
	int m_driverIdCounter;
};
