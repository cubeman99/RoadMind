#ifndef _BIARC_H_
#define _BIARC_H_

#include <cmgMath/cmg_math.h>

// http://www.ryanjuckett.com/programming/biarc-interpolation/

enum class BiarcType
{
	SINGLE,
	DOUBLE,
	STRAIGHT,
};

// Information about an arc used in biarc interpolation. Use 
// Vec_BiarcInterp_ComputeArcs to compute the values and use Vec_BiarcInterp
// to interpolate along the arc pair.
struct Biarc
{
	Vector2f	center;
	Vector2f	start;
	Vector2f	end;
	float		radius;		// radius of the circle (zero for lines)
	float		angle;		// angle to rotate from axis1 towards axis2
	float		length;		// distance along the arc

	Biarc()
		: center(Vector2f::ZERO)
		, start(Vector2f::ZERO)
		, end(Vector2f::ZERO)
		, radius(0.0f)
		, angle(0.0f)
		, length(0.0f)
	{
	}

	inline static Biarc CreatePoint(const Vector2f& point)
	{
		Biarc result;
		result.start = point;
		result.end = point;
		result.center = point;
		result.radius = 0.0f;
		result.length = 0.0f;
		result.angle = 0.0f;
		return result;
	}

	inline static Biarc CreateLine(const Vector2f& start, const Vector2f& end)
	{
		Biarc result;
		result.start = start;
		result.end = end;
		result.center = (start + end) * 0.5f;
		result.radius = 0.0f;
		result.length = start.DistTo(end);
		result.angle = 0.0f;
		return result;
	}

	inline bool IsStraight() const
	{
		return (radius == 0.0f);
	}

	inline Vector2f GetStartTangent() const
	{
		Vector2f normal = (start - center) / radius;
		normal = Vector2f(-normal.y, normal.x);
		if (angle < 0.0f)
			normal = -normal;
		return normal;
	}

	inline Vector2f GetEndTangent() const
	{
		Vector2f normal = (end - center) / radius;
		normal = Vector2f(-normal.y, normal.x);
		if (angle < 0.0f)
			normal = -normal;
		return normal;
	}

	inline Vector2f GetStartNormal() const
	{
		return ((start - center) / radius);
	}

	inline Vector2f GetEndNormal() const
	{
		return ((end - center) / radius);
	}

	inline Vector2f GetPoint(float distance) const
	{
		if (radius == 0.0f)
			return start + ((end - start) * (distance / length));
		else
		{
			Vector2f point = start;
			return point.Rotate(center, Math::Sign(angle) * (distance / radius));
		}
	}

	inline void CalcAngleAndLength(bool shortWay)
	{
		Vector2f normal = center - start;
		angle = (end - center).Dot(start - center) / (radius * radius);
		angle = Math::ACos(angle);
		if (!shortWay)
			angle = Math::TWO_PI - Math::Abs(angle);
		normal = Vector2f(-normal.y, normal.x);
		if (end.Dot(normal) > center.Dot(normal))
			angle = -angle;
		length = Math::Abs(angle) * radius;
	}

	inline Biarc Reverse() const
	{
		Biarc result = *this;
		result.start = end;
		result.end = start;
		result.angle = -angle;
		return result;
	}
};

// A pair of arcs
struct BiarcPair
{
public:
	union
	{
		struct
		{
			Biarc first;
			Biarc second;
		};
		struct
		{
			Biarc arcs[2];
		};
	};

	//-------------------------------------------------------------------------
	// Constructors
	//-------------------------------------------------------------------------

	BiarcPair();
	BiarcPair(const Biarc& first, const Biarc& second);

	//-------------------------------------------------------------------------
	// Operations
	//-------------------------------------------------------------------------

	float Length() const;
	BiarcPair Reverse() const;
	Vector2f GetPoint(float distance) const;

	//-------------------------------------------------------------------------
	// Static methods
	//-------------------------------------------------------------------------

	// Perform circular biarc interpolation between two points with directions.
	static BiarcPair Interpolate(
		const Vector2f& p1, const Vector2f& t1,
		const Vector2f& p2, const Vector2f& t2);
	static BiarcPair CreateParallel(const BiarcPair& biarcs, float offset);
	static BiarcPair CreateParallel(const BiarcPair& biarcs, float offset1, float offset2);

private:

	//-------------------------------------------------------------------------
	// Internal methods
	//-------------------------------------------------------------------------

	static BiarcPair CreateExpanding(const BiarcPair& base, float deltaOffset);
};


Vector2f ComputeCircleCenter(const Vector2f& p1, const Vector2f& t1, const Vector2f& p2);
Biarc ComputeArc(const Vector2f& p1, const Vector2f& t1, const Vector2f& p2);

// TODO: Remove these
void ComputeBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2, Biarc& arc1, Biarc& arc2);
void ComputeBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2,
	Vector2f& pm, Vector2f& q1, Vector2f& q, Biarc& arc1, Biarc& arc2, BiarcType& type);
Biarc CreateParallelBiarc(const Biarc& arc, float offset);
Biarc CreateParallelBiarc(const Biarc& arc, float startOffset, float endOffset);
void ComputeExpandingBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2,
	Vector2f midpoint, Vector2f midpointNormal, float offset, Biarc& outArc1, Biarc& outArc2);


#endif // _BIARC_H_