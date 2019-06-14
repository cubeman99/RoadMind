#ifndef _ROAD_CURVES_H_
#define _ROAD_CURVES_H_

#include "Biarc.h"


struct VerticalCurve
{
	float height1;
	float height2;
	
	float slope1;
	float slope2;
	float length;
	float a;
	float b;
	float offset;


	VerticalCurve();
	VerticalCurve(float h1, float h2);

	// Getters

	float GetStartHeight() const;
	float GetEndHeight() const;
	float GetHeight(float t) const;
	float GetHeightFromDistance(float distance) const;
	float GetSlope(float distance) const;
	VerticalCurve Reverse() const;

	// Setters

	// Cubic interpolation
	// y = ax^3 + bx^2 + (slope1)x + height1
	void CubicInterpolatation(float slope1, float slope2, float length);
	// Linear interpolation
	// y = (slope1)x + height1
	void LinearInterpolatation(float length);
	void Slice(float start);
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
	RoadCurveLine(const BiarcPair& horizontalCurve,
		float h1, float h2, float slope1, float slope2);
	float Length() const;
	Vector3f GetPoint(float distance) const;
	Vector3f Start() const;
	Vector3f Middle() const;
	Vector3f End() const;
	RoadCurveLine Reverse() const;
};



#endif // _ROAD_CURVES_H_