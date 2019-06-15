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
	, slope1(0.0f)
	, slope2(0.0f)
	, length(0.0f)
	, a(0.0f)
	, b(0.0f)
	, offset(0.0f)
{
}

VerticalCurve::VerticalCurve(float h1, float h2)
	: height1(h1)
	, height2(h2)
	, slope1(0.0f)
	, slope2(0.0f)
	, length(0.0f)
	, a(0.0f)
	, b(0.0f)
	, offset(0.0f)
{
}

VerticalCurve::VerticalCurve(float h1, float h2, float length, float slope1, float slope2)
	: height1(h1)
	, height2(h2)
	, slope1(slope1)
	, slope2(slope2)
	, length(length)
	, a(0.0f)
	, b(0.0f)
	, offset(0.0f)
{
	CubicInterpolatation(slope1, slope2, length);
}

float VerticalCurve::GetStartHeight() const
{
	return GetHeightFromDistance(0.0f);
}

float VerticalCurve::GetEndHeight() const
{
	return GetHeightFromDistance(length);
}

float VerticalCurve::GetHeight(float t) const
{
	//return Math::Lerp(height1, height2, Smooth(t));
	return Math::Lerp(height1, height2, t);
}

float VerticalCurve::GetHeightFromDistance(float distance) const
{
	distance = Math::Clamp(distance + offset, 0.0f, length);
	float x2 = distance * distance;
	float x3 = x2 * distance;
	return distance * (distance * (distance * a + b) + slope1) + height1;
}

float VerticalCurve::GetSlope(float distance) const
{
	distance = Math::Clamp(distance - offset, 0.0f, length);
	return distance * ((distance * 3.0f * a) + (2.0f * b)) + slope1;
}


VerticalCurve VerticalCurve::Reverse() const
{
	return VerticalCurve(height2, height1);
}

// Cubic interpolation
// y = ax^3 + bx^2 + (slope1)x + height1
void VerticalCurve::CubicInterpolatation(float slope1, float slope2, float length)
{
	this->length = length;
	if (length > FLT_EPSILON)
	{
		this->slope1 = slope1;
		this->slope2 = slope2;
		float h = height2 - height1;
		float d2 = length * length;
		float d3 = d2 * length;
		a = ((length * (slope1 + slope2)) - (2.0f * h)) / d3;
		b = (-(a * d3) - (slope1 * length) + h) / d2;
	}
	else
	{
		this->slope1 = 0.0f;
		this->slope2 = 0.0f;
		this->a = 0.0f;
		this->b = 0.0f;
	}
}

// Linear interpolation
// y = (slope1)x + height1
void VerticalCurve::LinearInterpolatation(float length)
{
	this->length = length;
	a = 0.0f;
	b = 0.0f;
	if (length > FLT_EPSILON)
	{
		slope1 = (height2 - height1) / length;
		slope2 = slope1;
	}
	else
	{
		slope1 = 0.0f;
		slope2 = 0.0f;
	}
}

void VerticalCurve::Slice(float start)
{
	offset = start;
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
	this->verticalCurve.LinearInterpolatation(horizontalCurve.Length());
}

RoadCurveLine::RoadCurveLine(const BiarcPair& horizontalCurve,
	float h1, float h2, float slope1, float slope2)
	: horizontalCurve(horizontalCurve)
	, verticalCurve(h1, h2)
	, t1(0.0f)
	, t2(1.0f)
{
	this->verticalCurve.CubicInterpolatation(
		slope1, slope2, horizontalCurve.Length());
}


float RoadCurveLine::Length() const
{
	return horizontalCurve.Length();
}

Vector3f RoadCurveLine::GetPoint(float distance) const
{
	Vector3f point;
	point.xy = horizontalCurve.GetPoint(distance);
	point.z = verticalCurve.GetHeightFromDistance(distance);
	return point;
}

Vector3f RoadCurveLine::GetTangent(float distance) const
{
	Vector3f normal;
	float slope = verticalCurve.GetSlope(distance);
	normal.xy = horizontalCurve.GetTangent(distance);
	normal.z = slope;
	normal.Normalize();
	return normal;
}

Vector3f RoadCurveLine::GetNormal(float distance) const
{
	Vector3f normal;
	float slope = verticalCurve.GetSlope(distance);
	normal.xy = horizontalCurve.GetTangent(distance) * -slope;
	normal.z = 1.0f;
	normal.Normalize();
	return normal;
}


Vector3f RoadCurveLine::Start() const
{
	Vector3f point;
	point.xy = horizontalCurve.first.start;
	point.z = verticalCurve.GetStartHeight();
	return point;
}

Vector3f RoadCurveLine::Middle() const
{
	Vector3f point;
	point.xy = horizontalCurve.first.end;
	point.z = verticalCurve.GetHeightFromDistance(
		horizontalCurve.first.length);
	return point;
}

Vector3f RoadCurveLine::End() const
{
	Vector3f point;
	point.xy = horizontalCurve.second.end;
	point.z = verticalCurve.GetEndHeight();
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

