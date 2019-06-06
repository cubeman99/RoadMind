#pragma once

#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include "CommonTypes.h"
#include "NodeGroup.h"
#include <set>

class Driver;


class RoadSurface
{
public:
	friend class RoadNetwork;

public:
	// Constructors

	RoadSurface();
	~RoadSurface();

	// Getters

	Set<Driver*>& GetDrivers();

	// Setters

	void AddDriver(Driver* driver);
	void RemoveDriver(Driver* driver);

	virtual void UpdateGeometry() = 0;

protected:
	Set<Driver*> m_drivers;
};

