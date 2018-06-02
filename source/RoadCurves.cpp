#include "RoadCurves.h"


static float Smooth(float t)
{
	if (t <= 0.5f)
		return (2.0f * t * t);
	else
		return (1.0f - 2.0f * (t - 1.0f) * (t - 1.0f));
}

VerticalCurve::VerticalCurve()
	: height1(0.0f)
	, height2(0.0f)
{
}

VerticalCurve::VerticalCurve(float h1, float h2)
	: height1(h1)
	, height2(h2)
{
}

float VerticalCurve::GetHeight(float t) const
{
	return Math::Lerp(height1, height2, Smooth(t));
}

VerticalCurve VerticalCurve::Reverse() const
{
	return VerticalCurve(height2, height1);
}


RoadCurveLine::RoadCurveLine()
	: t1(0.0f)
	, t2(1.0f)
{
}

RoadCurveLine::RoadCurveLine(const BiarcPair& horizontalCurve)
	: horizontalCurve(horizontalCurve)
	, t1(0.0f)
	, t2(1.0f)
{
}

RoadCurveLine::RoadCurveLine(const BiarcPair& horizontalCurve, const VerticalCurve& verticalCurve)
	: horizontalCurve(horizontalCurve)
	, verticalCurve(verticalCurve)
	, t1(0.0f)
	, t2(1.0f)
{
}

RoadCurveLine::RoadCurveLine(const BiarcPair& horizontalCurve, float h1, float h2)
	: horizontalCurve(horizontalCurve)
	, verticalCurve(h1, h2)
	, t1(0.0f)
	, t2(1.0f)
{
}


float RoadCurveLine::Length() const
{
	return horizontalCurve.Length();
}

Vector3f RoadCurveLine::GetPoint(float distance) const
{
	Vector3f point;
	float t = distance / Length();
	point.xy = horizontalCurve.GetPoint(distance);
	point.z = verticalCurve.GetHeight(t);
	return point;
}


Vector3f RoadCurveLine::Start() const
{
	Vector3f point;
	point.xy = horizontalCurve.first.start;
	point.z = verticalCurve.GetHeight(t1);
	return point;
}

Vector3f RoadCurveLine::Middle() const
{
	Vector3f point;
	point.xy = horizontalCurve.first.end;
	point.z = verticalCurve.GetHeight(
		Math::Lerp(t1, t2, horizontalCurve.first.length /
		(horizontalCurve.first.length + horizontalCurve.second.length)));
	return point;
}

Vector3f RoadCurveLine::End() const
{
	Vector3f point;
	point.xy = horizontalCurve.second.end;
	point.z = verticalCurve.GetHeight(t2);
	return point;
}

RoadCurveLine RoadCurveLine::Reverse() const
{
	RoadCurveLine result;
	result.horizontalCurve = horizontalCurve.Reverse();
	result.verticalCurve = verticalCurve.Reverse();
	result.t1 = t2;
	result.t2 = t1;
	return result;
}

