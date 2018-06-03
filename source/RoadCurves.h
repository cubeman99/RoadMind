#ifndef _ROAD_CURVES_H_
#define _ROAD_CURVES_H_

#include "Biarc.h"


struct VerticalCurve
{
	float height1;
	float height2;
	
	VerticalCurve();
	VerticalCurve(float h1, float h2);
	float GetHeight(float t) const;
	VerticalCurve Reverse() const;
};


struct RoadCurveLine
{
	BiarcPair horizontalCurve;
	VerticalCurve verticalCurve;
	float t1;
	float t2;

	RoadCurveLine();
	explicit RoadCurveLine(const BiarcPair& horizontalCurve);
	RoadCurveLine(const BiarcPair& horizontalCurve, const VerticalCurve& verticalCurve);
	RoadCurveLine(const BiarcPair& horizontalCurve, float h1, float h2);
	float Length() const;
	Vector3f GetPoint(float distance) const;
	Vector3f Start() const;
	Vector3f Middle() const;
	Vector3f End() const;
	RoadCurveLine Reverse() const;
};



#endif // _ROAD_CURVES_H_