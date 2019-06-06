#include "RoadSurface.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

RoadSurface::RoadSurface()
{
}

RoadSurface::~RoadSurface()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

Set<Driver*>& RoadSurface::GetDrivers()
{
	return m_drivers;
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void RoadSurface::AddDriver(Driver* driver)
{
	m_drivers.insert(driver);
}

void RoadSurface::RemoveDriver(Driver* driver)
{
	m_drivers.erase(driver);
}
