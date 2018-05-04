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

BiarcPair BiarcPair::Interpolate(
	const Vector2f& p1, const Vector2f& t1,
	const Vector2f& p2, const Vector2f& t2)
{
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
	if (denom == 0.0f)
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
	result.first = CreateParallelBiarc(biarcs.first, offset);
	result.second = CreateParallelBiarc(biarcs.second, offset);
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

	Vector2f startTangent = base.first.GetStartTangent();
	Vector2f midNormal = base.second.GetStartTangent();
	midNormal = Vector2f(-midNormal.y, midNormal.x);
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
		Vector2f e2 = result.second.GetStartTangent();
		e2 = Vector2f(-e2.y, e2.x);
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

	Vector2f n = (a - q);
	n = Vector2f(-n.y, n.x);
	if (a.Dot(n) < b.Dot(n))
		arc.angle = -arc.angle;
	if (d < 0.0f)
		arc.angle = -arc.angle;
	return arc;
}

void ComputeArcAngle(Biarc& arc, float d)
{
	Vector2f normal = arc.center - arc.start;
	arc.angle = (arc.end - arc.center).Dot(arc.start - arc.center) / (arc.radius * arc.radius);
	arc.angle = Math::ACos(arc.angle);
	if (d < 0.0f)
		arc.angle = Math::TWO_PI - Math::Abs(arc.angle);
	normal = Vector2f(-normal.y, normal.x);
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

Biarc CreateParallelBiarc(const Biarc& arc, float offset)
{
	Biarc result = arc;

	Vector2f normal = arc.center - arc.start;
	normal = Vector2f(-normal.y, normal.x);
	if (arc.end.Dot(normal) < arc.center.Dot(normal))
		offset = -offset;

	float scale = (arc.radius + offset) / arc.radius;
	result.radius = arc.radius * scale;
	result.start = arc.center + (arc.start - arc.center) * scale;
	result.end = arc.center + (arc.end - arc.center) * scale;
	result.length = result.radius * Math::Abs(result.angle);
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


Biarc CreateParallelBiarc(const Biarc& arc, float startOffset, float endOffset)
{
	Biarc result = arc;

	Vector2f normal = arc.center - arc.start;
	normal = Vector2f(-normal.y, normal.x);
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
		Vector2f e2 = outArc2.GetStartTangent();
		e2 = Vector2f(-e2.y, e2.x);

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


