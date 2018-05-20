#include "Biarc.h"

static void ComputeArcAngle(Biarc& arc, float d);


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

BiarcPair::BiarcPair()
{
}

BiarcPair::BiarcPair(const Biarc& first, const Biarc& second)
	: first(first)
	, second(second)
{
	this->first = first;
	this->second = second;
}

BiarcPair::BiarcPair(const Biarc& arc)
{
	this->first = arc;
	this->second = Biarc::CreatePoint(arc.end);
}


//-----------------------------------------------------------------------------
// Operations
//-----------------------------------------------------------------------------

float BiarcPair::Length() const
{
	return (first.length + second.length);
}

BiarcPair BiarcPair::Reverse() const
{
	BiarcPair result;
	result.first = second.Reverse();
	result.second = first.Reverse();
	return result;
}

Vector2f BiarcPair::GetPoint(float distance) const
{
	if (distance < first.length)
		return first.GetPoint(distance);
	else
		return second.GetPoint(distance - first.length);
}


//-----------------------------------------------------------------------------
// Static Methods
//-----------------------------------------------------------------------------

BiarcPair BiarcPair::CreatePoint(const Vector2f& point)
{
	BiarcPair result;
	result.first = Biarc::CreatePoint(point);
	result.second = result.first;
	return result;
}

BiarcPair BiarcPair::Split(const Biarc& arc)
{
	BiarcPair result;
	result.first = arc;
	result.first.angle *= 0.5f;
	result.first.length *= 0.5f;
	result.second = arc;
	result.second.angle *= 0.5f;
	result.second.length *= 0.5f;
	result.first.end = arc.GetMidPoint();
	result.second.start = result.first.end;
	return result;
}

BiarcPair BiarcPair::Interpolate(
	const Vector2f& p1, const Vector2f& t1,
	const Vector2f& p2, const Vector2f& t2)
{
	const float epsilon = 0.001f;

	BiarcPair result;
	Vector2f pm, q1, q2;
	Vector2f n1(-t1.y, t1.x);
	Vector2f n2(-t2.y, t2.x);
	Vector2f v = p2 - p1;
	Vector2f t = t1 + t2;

	float numer = -v.Dot(t) + Math::Sqrt(v.Dot(t) * v.Dot(t) +
		2.0f * (1.0f - t1.Dot(t2)) * (v.Dot(v)));
	float denom = 2.0f * (1.0f - t1.Dot(t2));	

	// Calculate the value of d2 such that d1 equals d2
	float d2;
	if (Math::Abs(denom) < epsilon)
		d2 = v.LengthSquared() / (4.0f * v.Dot(t2));
	else
		d2 = numer / denom;
	float d1 = d2;

	// Calculate q1, q2, and pm
	q1 = p1 + (d1 * t1);
	q2 = p2 - (d2 * t2);
	float w1 = d1 / (d1 + d2);
	float w2 = d2 / (d1 + d2);
	pm = q2 + Vector2f::Normalize(q1 - q2) * d2;

	// Compute arc1 center and radius
	result.first.start = p1;
	result.first.end = pm;
	denom = (2 * n1).Dot(pm - p1);
	if (Math::Abs(denom) > 0.01f)
	{
		float s1 = pm.DistToSqr(p1) / denom;
		result.first.center = p1 + (n1 * s1);
		result.first.radius = Math::Abs(s1);
		ComputeArcAngle(result.first, d1);
	}
	else
	{
		// The arc is a straight line!
		result.first.radius = 0.0f;
		result.first.angle = 0.0f;
		result.first.length = result.first.start.DistTo(result.first.end);
	}

	// Compute arc2 center and radius
	result.second.start = pm;
	result.second.end = p2;
	denom = (2 * n2).Dot(pm - p2);
	if (Math::Abs(denom) > 0.01f)
	{
		float s2 = pm.DistToSqr(p2) / denom;
		result.second.center = p2 + (n2 * s2);
		result.second.radius = Math::Abs(s2);
		ComputeArcAngle(result.second, d2);
	}
	else
	{
		// The arc is a straight line
		result.second.radius = 0.0f;
		result.second.angle = 0.0f;
		result.second.length = result.second.start.DistTo(result.second.end);
	}

	return result;
}

BiarcPair BiarcPair::CreateParallel(const BiarcPair& biarcs, float offset)
{
	BiarcPair result;
	result.first = Biarc::CreateParallel(biarcs.first, offset);
	result.second = Biarc::CreateParallel(biarcs.second, offset);
	return result;
}

BiarcPair BiarcPair::CreateParallel(const BiarcPair& biarcs, float offset1, float offset2)
{
	BiarcPair result;
	if (offset1 == offset2)
	{
		return CreateParallel(biarcs, offset1);
	}
	else if (offset2 < offset1)
	{
		BiarcPair base = CreateParallel(biarcs.Reverse(), -offset2);
		return CreateExpanding(base, offset2 - offset1).Reverse();
	}
	else
	{
		BiarcPair base = CreateParallel(biarcs, offset1);
		return CreateExpanding(base, offset2 - offset1);
	}
}


//-----------------------------------------------------------------------------
// Internal Methods
//-----------------------------------------------------------------------------

BiarcPair BiarcPair::CreateExpanding(const BiarcPair& base, float offset)
{
	BiarcPair result;

	if (base.first.IsStraight() && base.second.IsStraight())
	{
		return BiarcPair::Interpolate(
			base.first.start,
			base.first.GetStartTangent(),
			base.second.end + (base.second.GetDirection() * offset),
			base.second.GetEndTangent());
	}

	Vector2f startTangent = base.first.GetStartTangent();
	Vector2f midNormal = RightPerpendicular(base.second.GetStartTangent());
	Vector2f endTangent = base.second.GetEndTangent();
	Vector2f endNormal(-endTangent.y, endTangent.x);
	Vector2f endPoint = base.second.end + (offset * endNormal);

	float t = 0.5f;
	float low = 0.0f;
	float high = 1.0f;
	int maxIterations = 10;

	// Iterively solve the expanding contour problem so that
	// both arcs meet on a shared tangent point, which is located on a
	// line perpendicular from the midpoint
	for (int i = 0; i < maxIterations; i++)
	{
		float w = offset * t;
		Vector2f passThrough = base.first.end + (midNormal * w);
		result.first = ComputeArc(base.first.start, startTangent, passThrough);
		result.second = ComputeArc(endPoint, -endTangent, passThrough).Reverse();

		Vector2f e1 = result.first.GetEndTangent();
		Vector2f e2 = RightPerpendicular(result.second.GetStartTangent());
		float error = e1.Dot(e2);
		if (offset < 0)
			error = -error;

		if (Math::Abs(error) < 0.0000001f)
		{
			return result;
		}
		else if (error < 0.0f)
		{
			low = t;
			t = (low + high) * 0.5f;
		}
		else
		{
			high = t;
			t = (low + high) * 0.5f;
		}
	}
	return result;
}





static Biarc ComputeArc(Vector2f a, Vector2f b, Vector2f q, float d)
{
	Biarc arc;
	arc.start = a;
	arc.end = b;
	Vector2f m = (a + b) * 0.5f;
	float h = b.DistTo(m) * 0.5f;
	float x = q.DistTo(m);
	arc.radius = (h * d) / x;
	float y = Math::Sqrt((arc.radius * arc.radius) - (h * h));
	arc.center = q + (m - q) * ((x + y) / x);
	arc.radius = a.DistTo(arc.center);
	arc.angle = (b - arc.center).Dot(a - arc.center) / (arc.radius * arc.radius);
	arc.angle = Math::ACos(arc.angle);

	Vector2f n = RightPerpendicular(a - q);
	if (a.Dot(n) < b.Dot(n))
		arc.angle = -arc.angle;
	if (d < 0.0f)
		arc.angle = -arc.angle;
	return arc;
}

void ComputeArcAngle(Biarc& arc, float d)
{
	Vector2f normal = RightPerpendicular(arc.center - arc.start);
	arc.angle = (arc.end - arc.center).Dot(arc.start - arc.center) / (arc.radius * arc.radius);
	arc.angle = Math::ACos(arc.angle);
	if (d < 0.0f)
		arc.angle = Math::TWO_PI - Math::Abs(arc.angle);
	if (arc.end.Dot(normal) > arc.center.Dot(normal))
		arc.angle = -arc.angle;
	arc.length = Math::Abs(arc.angle) * arc.radius;
}

void ComputeBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2, Biarc& arc1, Biarc& arc2)
{
	Vector2f pm, q1, q2;
	BiarcType type;
	ComputeBiarcs(p1, t1, p2, t2, pm, q1, q2, arc1, arc2, type);
}

void ComputeBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2,
	Vector2f& pm, Vector2f& q1, Vector2f& q2, Biarc& arc1, Biarc& arc2, BiarcType& type)
{
	Vector2f intersection = Line2f::GetLineIntersection(
		p1, p1 + t1, p2, p2 + t2);
	if ((intersection.Dot(t1) > p1.Dot(t1)) ==
		(intersection.Dot(t2) < p2.Dot(t2)))
		type = BiarcType::DOUBLE;
	else
		type = BiarcType::SINGLE;
	pm = intersection;

	Vector2f n1(-t1.y, t1.x);
	Vector2f n2(-t2.y, t2.x);
	Vector2f v = p2 - p1;
	Vector2f t = t1 + t2;

	float numer = -v.Dot(t) + Math::Sqrt(v.Dot(t) * v.Dot(t) + 2.0f * (1.0f - t1.Dot(t2)) * (v.Dot(v)));
	float denom = 2.0f * (1.0f - t1.Dot(t2));

	float d2;
	if (denom == 0.0f)
		d2 = v.LengthSquared() / (4.0f * v.Dot(t2));
	else
		d2 = numer / denom;
	float d1 = d2;

	q1 = p1 + (d1 * t1);
	q2 = p2 - (d2 * t2);
	float w1 = d1 / (d1 + d2);
	float w2 = d2 / (d1 + d2);
	pm = q2 + Vector2f::Normalize(q1 - q2) * d2;

	// Compute arc1 center and radius
	arc1.start = p1;
	arc1.end = pm;
	denom = (2 * n1).Dot(pm - p1);
	if (Math::Abs(denom) > 0.01f)
	{
		float s1 = pm.DistToSqr(p1) / denom;
		arc1.center = p1 + (n1 * s1);
		arc1.radius = Math::Abs(s1);
		ComputeArcAngle(arc1, d1);
	}
	else
	{
		// The arc is a straight line!
		arc1.radius = 0.0f;
		arc1.angle = 0.0f;
		arc1.length = arc1.start.DistTo(arc1.end);
	}

	// Compute arc2 center and radius
	arc2.start = pm;
	arc2.end = p2;
	denom = (2 * n2).Dot(pm - p2);
	if (Math::Abs(denom) > 0.01f)
	{
		float s2 = pm.DistToSqr(p2) / denom;
		arc2.center = p2 + (n2 * s2);
		arc2.radius = Math::Abs(s2);
		ComputeArcAngle(arc2, d2);
	}
	else
	{
		// The arc is a straight line
		arc2.radius = 0.0f;
		arc2.angle = 0.0f;
		arc2.length = arc2.start.DistTo(arc2.end);
	}

	return;

	if (type == BiarcType::DOUBLE)
	{
		float dist1 = intersection.Dot(t1) - p1.Dot(t1);
		float dist2 = p2.Dot(t2) - intersection.Dot(t2);
		float dist = Math::Min(dist1, dist2);

		Vector2f a1, a2;

		Biarc* curvedArc = (dist2 < dist1 ? &arc2 : &arc1);
		Biarc* straightArc = (dist2 < dist1 ? &arc1 : &arc2);

		if (dist2 < dist1)
		{
			a1 = intersection - (t1 * dist2);
			a2 = p2;
			arc1.end = a1;
		}
		else
		{
			a1 = p1;
			a2 = intersection + (t2 * dist1);
			arc2.start = a2;
			arc2.end = p2;
		}

		Vector2f center = Line2f::GetLineIntersection(
			a1, a1 + n1, a2, a2 + n2);
		float radius = a1.DistTo(center);

		bool flip = (a2.Dot(n1) < intersection.Dot(n1));

		straightArc->radius = 0.0f;
		curvedArc->start = a1;
		curvedArc->end = a2;
		curvedArc->radius = radius;
		curvedArc->center = center;
		ComputeArcAngle(*curvedArc, dist);
		//if (flip)
		//curvedArc->angle = -curvedArc->angle;
	}

	//arc1 = ComputeArc(p1, pm, q1, d1);
	//arc2 = ComputeArc(pm, p2, q2, d2);
}

Biarc Biarc::CreateParallel(const Biarc& arc, float offset)
{
	Biarc result = arc;

	if (arc.IsStraight())
	{
		Vector2f right = RightPerpendicular((arc.end - arc.start) / arc.length);
		Vector2f move = right * offset;
		result.start += move;
		result.end += move;
		result.center += move;
	}
	else
	{
		Vector2f normal = RightPerpendicular(arc.center - arc.start);
		if (arc.end.Dot(normal) < arc.center.Dot(normal))
			offset = -offset;

		float scale = (arc.radius + offset) / arc.radius;
		result.radius = arc.radius * scale;
		result.start = arc.center + (arc.start - arc.center) * scale;
		result.end = arc.center + (arc.end - arc.center) * scale;
		result.length = result.radius * Math::Abs(result.angle);
	}

	return result;
}

// Calculate the center of a circle from two points and a tangent
Vector2f ComputeCircleCenter(const Vector2f& p1, const Vector2f& t1, const Vector2f& p2)
{
	Vector2f v = p2 - p1;
	Vector2f normal(-t1.y, t1.x);
	Vector2f bisector(-v.y, v.x);
	Vector2f midpoint = (p1 + p2) * 0.5f;
	Vector2f center = Line2f::GetLineIntersection(
		p1, p1 + normal, midpoint, midpoint + bisector);
	return center;
}

Biarc ComputeArc(const Vector2f& p1, const Vector2f& t1, const Vector2f& p2)
{
	Vector2f v = p2 - p1;
	Vector2f normal(-t1.y, t1.x);
	Vector2f bisector(-v.y, v.x);
	Vector2f midpoint = (p1 + p2) * 0.5f;

	Biarc result;
	result.center = Line2f::GetLineIntersection(
		p1, p1 + normal, midpoint, midpoint + bisector);
	result.start = p1;
	result.end = p2;
	result.radius = result.center.DistTo(p1);
	float d = 1.0f;

	if (p2.Dot(t1) < p1.Dot(t1))
		d = -1.0f;
	ComputeArcAngle(result, d);
	if (d < 0.0f)
		result.angle = -result.angle;
	return result;
}


Biarc Biarc::CreateParallel(const Biarc& arc, float startOffset, float endOffset)
{
	Biarc result = arc;

	Vector2f normal = RightPerpendicular(arc.center - arc.start);
	if (arc.end.Dot(normal) < arc.center.Dot(normal))
	{
		startOffset = -startOffset;
		endOffset = -endOffset;
	}
	float startScale = (arc.radius + startOffset) / arc.radius;
	float endScale = (arc.radius + endOffset) / arc.radius;
	result.start = arc.center + (arc.start - arc.center) * startScale;
	result.end = arc.center + (arc.end - arc.center) * endScale;
	result.center = ComputeCircleCenter(result.start, arc.GetStartTangent(), result.end);
	result.radius = result.start.DistTo(result.center);
	ComputeArcAngle(result, 1.0f);
	return result;
}


//Biarc ComputeExpandingBiarcs(const Biarc& arc1, const Biarc& arc2, Biarc& outArc1, Biarc& outArc2, float offset)
void ComputeExpandingBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2,
	Vector2f midpoint, Vector2f midpointNormal, float offset, Biarc& outArc1, Biarc& outArc2)
{
	Vector2f n2(-t2.y, t2.x);
	Vector2f p3 = p2 + (offset * n2);

	float t = 0.5f;
	float low = 0.0f;
	float high = 1.0f;
	int maxIterations = 10;

	// Iterively solve the expanding contour problem so that
	// both arcs meet on a shared tangent point, which is located on a
	// line perpendicular from the midpoint
	for (int i = 0; i < maxIterations; i++)
	{
		float w = offset * t;
		Vector2f passThrough = midpoint + (midpointNormal * w);
		outArc1 = ComputeArc(p1, t1, passThrough);
		outArc2 = ComputeArc(p3, -t2, passThrough).Reverse();

		Vector2f e1 = outArc1.GetEndTangent();
		Vector2f e2 = RightPerpendicular(outArc2.GetStartTangent());

		float error = e1.Dot(e2);
		if (offset < 0)
			error = -error;

		if (Math::Abs(error) < 0.0000001f)
		{
			return;
		}
		else if (error < 0.0f)
		{
			low = t;
			t = (low + high) * 0.5f;
		}
		else
		{
			high = t;
			t = (low + high) * 0.5f;
		}
	}
}









static Vector2f CalcCircle(float b, float A, float B, float r, float R,
	bool inside1 = false, bool inside2 = false)
{
	float br = b + r;
	float BR = B + R;
	if (inside1)
		br = b - r;
	if (inside2)
		BR = B - R;
	float br2 = br * 2;
	float BR2 = BR * 2;
	float discriminant = (A*A) / (BR*BR) - (4 * (1 / br2 - 1 / BR2) *
		((b*b - r*r) / br2 + (R*R - A*A - B*B) / BR2));
	float x = (Math::Sqrt(discriminant) - A / BR) / (2 * (1 / br2 - 1 / BR2));
	float y = (x*x + b*b - r*r) / br2;
	return Vector2f(x, y);
}

// p1 is to the left of p2
static Biarc CalcWebbedCircle(Vector2f p1, Vector2f p2, Vector2f c1, Vector2f c2)
{
	// Compute the circle tangent to two circles and a line
	Vector2f xAxis = (p2 - p1).Normalize();
	Vector2f yAxis(-xAxis.y, xAxis.x);
	float xOrigin = c1.Dot(xAxis);
	float yOrigin = p1.Dot(yAxis);
	float b = c1.Dot(yAxis) - yOrigin;
	float A = c2.Dot(xAxis) - xOrigin;
	float B = c2.Dot(yAxis) - yOrigin;
	float r = c1.DistTo(p1);
	float R = c2.DistTo(p2);
	bool inside1 = (c1.Dot(xAxis) > p1.Dot(xAxis));
	bool inside2 = (c2.Dot(xAxis) < p2.Dot(xAxis));
	Vector2f c = CalcCircle(b, A, B, r, R, inside1, inside2);
	Vector2f center = c1 + (c.x * xAxis) + ((c.y - b) * yAxis);
	float radius = c.y;

	// Create an arc from the resulting circle
	Vector2f w1 = center + (c1 - center).Normalize() * radius * (inside1 ? -1.0f : 1.0f);
	Vector2f w2 = center - (yAxis * radius);
	Vector2f w3 = center + (c2 - center).Normalize() * radius * (inside2 ? -1.0f : 1.0f);
	//BiarcPair pair;
	//pair.first = Biarc::CreateArc(w1, w2, center, true);
	//pair.second = Biarc::CreateArc(w2, w3, center, true);
	return Biarc::CreateArc(w1, w3, center, true);
}


static float CalcTangentCircle(float b, float R, float r, bool inside)
{
	float discriminant;
	if (inside)
		discriminant = ((R - r) * (R - r)) - ((r - b) * (r - b));
	else
		discriminant = ((R + r) * (R + r)) - ((r - b) * (r - b));
	float x = Math::Sqrt(discriminant);
	if (inside)
		x = -x;
	return x;
}

static Biarc CalcTangentCircle(Vector2f c, Vector2f p1, Vector2f p2, float radius, bool left)
{
	// Compute the circle tangent to two circles and a line
	Vector2f xAxis = (p2 - p1).Normalize();
	Vector2f yAxis(-xAxis.y, xAxis.x);
	if (left)
		yAxis = -yAxis;
	float xOrigin = c.Dot(xAxis);
	float yOrigin = p1.Dot(yAxis);
	float b = c.Dot(yAxis) - yOrigin;
	float R = c.DistTo(p1);
	bool inside = (c.Dot(xAxis) > p1.Dot(xAxis));
	float x = CalcTangentCircle(b, R, radius, inside);
	Vector2f center = c + (x * xAxis) + ((radius - b) * yAxis);
	Vector2f start = center + Vector2f::Normalize(c - center) * radius;
	Vector2f end = center - (yAxis * radius);
	if (inside)
		start = center - Vector2f::Normalize(c - center) * radius;
	Biarc result = Biarc::CreateArc(start, end, center, !left);
	return result;
}
Convexity GetConvexity(const Vector2f& a, const Vector2f& b, const Vector2f& c)
{
	Vector2f left = LeftPerpendicular(b - a);
	float diff = c.Dot(left) - b.Dot(left);
	float eps = 0.00000001f;
	if (diff < -eps)
		return Convexity::CONVEX;
	if (diff > eps)
		return Convexity::CONCAVE;
	else
		return Convexity::STRAIGHT;
}

bool IsConvex(const Vector2f& a, const Vector2f& b, const Vector2f& c)
{
	Vector2f left = LeftPerpendicular(b - a);
	return (c.Dot(left) < b.Dot(left));
}

bool IsInsideTriangle(const Vector2f& a, const Vector2f& b, const Vector2f& c, const Vector2f& v)
{
	Vector2f n1 = LeftPerpendicular(b - a);
	Vector2f n2 = LeftPerpendicular(c - b);
	Vector2f n3 = LeftPerpendicular(a - c);
	const float eps = 0.00001f;
	return (v.Dot(n1) < a.Dot(n1) - eps &&
		v.Dot(n2) < b.Dot(n2) - eps &&
		v.Dot(n3) < c.Dot(n3) - eps);
}

static bool CastRayOnArc(const Vector2f& origin, const Vector2f& direction, const Biarc& arc, bool inside, Vector2f& intersection)
{
	Vector2f yAxis(direction.y, -direction.x);
	float x = arc.center.Dot(direction) - origin.Dot(direction);
	float y = arc.center.Dot(yAxis) - origin.Dot(yAxis);
	if (Math::Abs(y) > arc.radius)
		return false;
	float w = Math::Sqrt((arc.radius * arc.radius) - (y * y));
	if (inside)
		w = -w;
	intersection = arc.center - (y * yAxis) - (w * direction);
	return true;
}

BiarcPair CalcWebbedCircle(const BiarcPair& a, const BiarcPair& b, float radius)
{
	Vector2f p1, p2, t1, t2;
	p1 = a.second.end;
	p2 = b.second.end;
	t1 = a.second.GetEndTangent();
	t2 = b.second.GetEndTangent();

	Vector2f n1 = LeftPerpendicular(t1);
	Vector2f n2 = RightPerpendicular(t2);
	if ((p1.Dot(n2) < p2.Dot(n2)) != (p2.Dot(n1) < p1.Dot(n1)))
		return BiarcPair::Interpolate(p1, t1, p2, -t2);

	Vector2f intersection = Line2f::GetLineIntersection(
		p1, p1 + t1, p2, p2 + t2);
	if (!IsConvex(p1, intersection, p2))
		return CalcWebbedCircle(b, a, radius).Reverse();

	// TODO: parallel case
	// TODO: >180 case

	float dist1 = p1.DistTo(intersection);
	float dist2 = p2.DistTo(intersection);

	float angle = -Math::ATan2(-t2.Dot(n1), -t2.Dot(t1));
	if (angle < 0.0f)
		angle += Math::TWO_PI;

	if (angle > Math::PI)
	{
		//dist1 = 
	}

	BiarcPair result;

	if (dist1 <= dist2)
	{
		float radius = dist1 * Math::Tan((Math::PI - angle) * 0.5f);
		Vector2f center = p1 + (RightPerpendicular(t1) * radius);
		Vector2f end = intersection - (t2 * dist1);
		result.first = Biarc::CreateArc(p1, end, center, true);
		result.second = Biarc::CreateLine(end, p2);
	}
	else
	{
		float radius = dist2 * Math::Tan((Math::PI - angle) * 0.5f);
		Vector2f center = p2 + (LeftPerpendicular(t2) * radius);
		Vector2f start = intersection - (t1 * dist2);
		result.second = Biarc::CreateArc(start, p2, center, true);
		result.first = Biarc::CreateLine(p1, start);
	}

	return result;

	/*
	bool convexA = IsConvex(a.second.end - a.second.GetEndTangent(), a.second.end, b.second.end);
	bool convexB = IsConvex(a.second.end, b.second.end, b.second.end - b.second.GetEndTangent());
	
	Vector2f xAxis = (b.second.end - a.second.end).Normalize();
	Vector2f yAxis(-xAxis.y, xAxis.x);
	Vector2f center, start, end, intersection;
	float gap;

	if (convexA && convexB)
	{
		BiarcPair pair = BiarcPair::Interpolate(a.second.end,
			a.second.GetEndTangent(), b.second.end, -b.second.GetEndTangent());
		web1 = pair.first;
		web2 = pair.second.Reverse();
		return;
	}
	else if (convexA)
	{
		Vector2f left = a.second.GetEndTangent();
		left = Vector2f(left.y, -left.x);
		start = a.second.end;
		center = start + (left * radius);
		end = center + (yAxis * radius);
		Vector2f offset = -yAxis * ((a.second.end.Dot(yAxis) - end.Dot(yAxis)));
		
		CastRayOnArc(a.second.end + offset, xAxis, b.second, b.second.angle < 0.0f, intersection);

		web1 = Biarc::CreateArc(start, end, center, false);
		web2 = CalcTangentCircle(b.second.center, intersection, web1.end, radius, true);
		gap = web2.end.Dot(xAxis) - web1.end.Dot(xAxis);

		if (gap < 0)
		{
			BiarcPair pair = BiarcPair::Interpolate(web1.start,
				web1.GetStartTangent(), web2.start, -web2.GetStartTangent());
			web1 = pair.first;
			web2 = pair.second.Reverse();
		}
	}
	else if (convexB)
	{
		Vector2f right = RightPerpendicular(b.second.GetEndTangent());
		start = b.second.end;
		center = start + (right * radius);
		end = center + (yAxis * radius);
		Vector2f offset = -yAxis * ((b.second.end.Dot(yAxis) - end.Dot(yAxis)));
		
		CastRayOnArc(b.second.end + offset, xAxis, a.second, a.second.angle < 0.0f, intersection);

		web2 = Biarc::CreateArc(start, end, center, true);
		web1 = CalcTangentCircle(a.second.center, intersection, web2.end, radius, false);
		gap = web2.end.Dot(xAxis) - web1.end.Dot(xAxis);

		if (gap < 0)
		{
			BiarcPair pair = BiarcPair::Interpolate(web1.start,
				web1.GetStartTangent(), web2.start, -web2.GetStartTangent());
			web1 = pair.first;
			web2 = pair.second.Reverse();
		}
	}
	else
	{
		web1 = CalcTangentCircle(a.second.center, a.second.end, b.second.end, radius, false);
		web2 = CalcTangentCircle(b.second.center, b.second.end, a.second.end, radius, true);
		gap = web2.end.Dot(xAxis) - web1.end.Dot(xAxis);

		if (gap < 0.0f)
		{
			web1 = CalcWebbedCircle(a.second.end, b.second.end, a.second.center, b.second.center);
			BiarcPair pair = BiarcPair::Split(web1);
			web1 = pair.first;
			web2 = pair.second.Reverse();
		}
	}
	*/
}

Vector2f LeftPerpendicular(const Vector2f& v)
{
	return Vector2f(v.y, -v.x);
}

Vector2f RightPerpendicular(const Vector2f& v)
{
	return Vector2f(-v.y, v.x);
}

Line2f CalcOuterTangent(const Circle2f& a, const Circle2f& b)
{
	Vector2f xAxis = Vector2f::Normalize(b.center - a.center);
	Vector2f yAxis = LeftPerpendicular(xAxis);
	float x = b.center.Dot(xAxis) - a.center.Dot(xAxis);
	float angle = Math::HALF_PI - ((a.radius - b.radius) / x);
	Vector2f direction = (xAxis * Math::Cos(angle)) +
		(yAxis * Math::Sin(angle));
	return Line2f(a.center + (direction * a.radius),
		b.center + (direction * b.radius));
}
