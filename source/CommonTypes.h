#ifndef _COMMON_TYPES_H_
#define _COMMON_TYPES_H_


typedef float Real;
typedef Real Meters;
typedef Real Radians;
typedef Real Seconds;
typedef Real MetersPerSecond;


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


#endif // _COMMON_TYPES_H_