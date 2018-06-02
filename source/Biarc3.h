#ifndef _BIARC_3_H_
#define _BIARC_3_H_

#include <cmgMath/cmg_math.h>

// http://www.ryanjuckett.com/programming/biarc-interpolation/


// Information about an arc used in biarc interpolation. Use 
// Vec_BiarcInterp_ComputeArcs to compute the values and use Vec_BiarcInterp
// to interpolate along the arc pair.
struct Biarc3
{
	Vector3f center;
	Vector3f axis1; // vector from center to the end point
	Vector3f axis2; // vector from center edge perpendicular to axis1
	Vector3f start;
	Vector3f end;
	Vector3f axis;
	float radius;		// radius of the circle (zero for lines)
	float angle;		// angle to rotate from axis1 towards axis2
	float length;		// distance along the arc

	Biarc3();

	inline static Biarc3 CreateArc(const Vector2f& start,
		const Vector2f& end, const Vector2f& center, bool clockwise = true);

	static Biarc3 CreatePoint(const Vector3f& point);
	static Biarc3 CreateLine(const Vector3f& start, const Vector3f& end);

	//static Biarc3 CreateParallel(const Biarc3& arc, float offset);
	//static Biarc3 CreateParallel(const Biarc3& arc, float startOffset, float endOffset);
	
	static void Interpolate(
		const Vector3f& p1, const Vector3f& t1,
		const Vector3f& p2, const Vector3f& t2,
		Biarc3& arc1, Biarc3& arc2);

	inline bool IsStraight() const
	{
		return (radius == 0.0f);
	}

	inline bool IsPoint() const
	{
		return (length <= FLT_MIN);
	}

	Vector3f Start() const;
	Vector3f End() const;
	Vector3f GetAxis() const;

	/*
	inline Vector2f GetStartTangent() const
	{
		if (IsStraight())
		{
			return ((end - start) / length);
		}
		else
		{
			Vector2f normal = (start - center) / radius;
			normal = Vector2f(-normal.y, normal.x);
			if (angle < 0.0f)
				normal = -normal;
			return normal;
		}
	}

	inline Vector2f GetEndTangent() const
	{
		if (IsStraight())
		{
			return ((end - start) / length);
		}
		else
		{
			Vector2f normal = (end - center) / radius;
			normal = Vector2f(-normal.y, normal.x);
			if (angle < 0.0f)
				normal = -normal;
			return normal;
		}
	}

	inline Vector2f GetStartNormal() const
	{
		if (IsStraight())
		{
			Vector2f direction = (end - start) / length;
			return Vector2f(-direction.y, direction.x);
		}
		else
		{
			return ((start - center) / radius);
		}
	}

	inline Vector2f GetDirection() const
	{
		if (IsStraight())
			return GetStartNormal();
		else
			return ((end - center) / radius);
	}

	inline Vector2f GetPoint(float distance) const
	{
		if (length == 0.0f)
		{
			return start;
		}
		else if (radius == 0.0f)
		{
			return Vector2f::Lerp(start, end, distance / length);
		}
		else
		{
			Vector2f point = start;
			return point.Rotate(center, Math::Sign(angle) * (distance / radius));
		}
	}

	inline Vector2f GetMidPoint() const
	{
		return GetPoint(length * 0.5f);
	}

	inline void CalcAngleAndLength(bool shortWay)
	{
		if (radius == 0.0f)
		{
			angle = 0.0f;
			length = start.DistTo(end);
		}
		else
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
	}

	inline Biarc3 Reverse() const
	{
		Biarc3 result = *this;
		result.start = end;
		result.end = start;
		result.angle = -angle;
		return result;
	}
	*/
};

/*
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
	explicit BiarcPair(const Biarc& arc);

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
	static BiarcPair CreatePoint(const Vector2f& point);
	static BiarcPair Split(const Biarc& arc);
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
*/

/*
Vector2f ComputeCircleCenter(const Vector2f& p1, const Vector2f& t1, const Vector2f& p2);
Biarc ComputeArc(const Vector2f& p1, const Vector2f& t1, const Vector2f& p2);

// TODO: Remove these
void ComputeBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2, Biarc& arc1, Biarc& arc2);
void ComputeBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2,
	Vector2f& pm, Vector2f& q1, Vector2f& q, Biarc& arc1, Biarc& arc2, BiarcType& type);
void ComputeExpandingBiarcs(Vector2f p1, Vector2f t1, Vector2f p2, Vector2f t2,
	Vector2f midpoint, Vector2f midpointNormal, float offset, Biarc& outArc1, Biarc& outArc2);

BiarcPair CalcWebbedCircle(const BiarcPair& a, const BiarcPair& b, float radius);
*/


#endif // _BIARC_3_H_