#pragma once

#include <cmgMath/cmg_math.h>


typedef float Real;
typedef Real Seconds;
typedef Real Kilograms;
typedef Real Meters;
typedef Real Radians;
typedef Real Newtons;
typedef Real MetersPerSecond;
typedef Real MetersPerSecondSq;
typedef Real RadiansPerSecond;
typedef Real NewtonMeters;



// distance
#define inchesToCentimeters(x)	(2.54f)
#define inchesToMeters(x)		(0.0254f)
#define feetToMeters(x)			(0.3048f)

// speed
#define metersPerSecondToMph(x)	(x * 2.23694f)
#define mphToMetersPerSecond(x)	(x / 2.23694f)

// rotation speed
#define rpmToRadPerSec(x)		((x / 60.0f) * Math::TWO_PI)
#define radPerSecToRpm(x)		((x / Math::TWO_PI) * 60.0f)

// power
#define kilowattsToWatts(x)		(x * 1000.0f)
#define horsepowerToWatts(x)	(x * 0.00134102f)


#define BOOL2ASCII(x) (x ? "TRUE" : "FALSE")


struct RoadMetrics
{
public:

	Meters laneWidth;
	Meters dividerWidth;
	Meters dividerLength;
	Meters dividerGapLength;
	Meters stopLineWidth;
	Meters parkingSpaceWidth;
	Meters parkingSpaceLength;
};


enum class AxleSide
{
	LEFT = 0,
	RIGHT = 1,
};


enum class LaneDivider
{
	SOLID = 0,
	DASHED = 1,
};


enum class LaneSide
{
	NONE = -1,
	LEFT = 0,
	RIGHT = 1,

	CENTER = 2, // debug purposes
};


enum class InputOutput
{
	NONE = -1,
	INPUT = 0,
	OUTPUT = 1,
	BOTH = 2,
};

typedef InputOutput IOType;


enum class RightOfWay
{
	GIVE_WAY = -1,
	NONE = 0,
	RIGHT_OF_WAY = 1,
};


enum class TrafficLightSignal
{
	NONE = 0,
	GO = 1,
	STOP = 2,
	GO_YIELD = 3,
	STOP_SIGN = 4,
	YELLOW = 5,
};

class IPosition
{
public:
	IPosition() {}
	virtual const Vector3f& GetPosition() const = 0;
	virtual void SetPosition(const Vector3f& position) = 0;
};

