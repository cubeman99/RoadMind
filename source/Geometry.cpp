#include "Geometry.h"

void Geometry::ZipArcs(Array<VertexPosNorm>& outVertices,
	Array<unsigned int>& outIndices, const Array<VertexPosNorm>& left,
	const Array<VertexPosNorm>& right)
{
	const Array<VertexPosNorm>* sides[2] = { &left, &right };
	unsigned int head[2] = { 1, 0 };
	VertexPosNorm a, b, c;
	int side = 0;
	bool prevConvex = true;
	Vector3f mins = left[0].position;
	for (unsigned int axis = 0; axis < 2; axis++)
	{
		for (unsigned int i = 0; i < left.size(); i++)
			mins[axis] = Math::Min(mins[axis], left[i].position[axis]);
		for (unsigned int i = 0; i < right.size(); i++)
			mins[axis] = Math::Min(mins[axis], right[i].position[axis]);
	}

	while (head[0] < left.size() && head[1] < right.size())
	{
		int other = 1 - side;

		// Remove equivilant vertices
		a = sides[side]->at(head[side] - 1);
		b = sides[side]->at(head[side]);
		if (a.position.xy.DistToSqr(b.position.xy) < 0.001f)
		{
			head[side] += 1;
			continue;
		}
		a = sides[side]->at(head[side] - 1);
		b = sides[other]->at(head[other]);
		if (a.position.xy.DistToSqr(b.position.xy) < 0.001f)
		{
			head[side] += 1;
			continue;
		}

		// Define the triangle in clockwise order
		if (side == 0)
		{
			a = sides[other]->at(head[other]);
			b = sides[side]->at(head[side] - 1);
			c = sides[side]->at(head[side]);
		}
		else
		{
			a = sides[side]->at(head[side] - 1);
			b = sides[other]->at(head[other]);
			c = sides[side]->at(head[side]);
		}

		// If this triangle will be concave, then switch to the other side
		Convexity convexity = GetConvexity(a.position.xy, b.position.xy, c.position.xy);
		if (prevConvex && convexity == Convexity::CONCAVE)
		{
			head[side] -= 1;
			head[other] += 1;
			prevConvex = false;
			side = other;
			continue;
		}
		prevConvex = true;

		outIndices.push_back(outVertices.size());
		outVertices.push_back(a);
		outIndices.push_back(outVertices.size());
		outVertices.push_back(b);
		outIndices.push_back(outVertices.size());
		outVertices.push_back(c);

		// Switch to the other side
		if (head[other] < sides[other]->size() - 1)
			side = other;
		head[side] += 1;
	}

	// Draw the shape
	/*glBegin(GL_TRIANGLES);
	glColor4ubv(color.data());
	Vector2f texOffset = Vector2f::ZERO;
	for (unsigned int axis = 0; axis < 2; axis++)
	{
		if (mins[axis] < 0)
			texOffset[axis] += (int) (mins[axis] - 5);
	}
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		Vector2f texCoord = (vertices[i].xy / 5.0f) + texOffset;
		glTexCoord2fv(texCoord.v);
		glVertex3fv(vertices[i].v);
	}
	glEnd();*/
}

void Geometry::ZipArcs(Array<VertexPosNorm>& outVertices,
	Array<unsigned int>& outIndices, const Array<RoadCurveLine>& left,
	const Array<RoadCurveLine>& right)
{
	Array<VertexPosNorm> sideVertices[2];
	const Array<RoadCurveLine>* sides[2] = { &left, &right };

	for (int side = 0; side < 2; side++)
	{
		for (unsigned int i = 0; i < sides[side]->size(); i++)
		{
			const RoadCurveLine& curve = sides[side]->at(i);
			float dist = 0.0f;
			sideVertices[side].push_back(VertexPosNorm(
				curve.Start(),
				curve.GetNormal(0.0f)));
			for (unsigned int half = 0; half < 2; half++)
			{
				Biarc arc = curve.horizontalCurve.arcs[half];
				if (arc.IsPoint())
					continue;
				if (!arc.IsStraight() && !arc.IsPoint())
				{
					Vector2f v = arc.start;
					int count = (int)((Math::Abs(arc.angle) / Math::TWO_PI) * 50) + 2;
					for (int j = 1; j < count; j++)
					{
						float t = j / (float) count;
						float angle = -arc.angle * t;
						v = arc.start;
						v.Rotate(arc.center, angle);
						float distAlongCurve = dist + (arc.length * t);
						float z = curve.verticalCurve.GetHeightFromDistance(distAlongCurve);
						sideVertices[side].push_back(VertexPosNorm(
							Vector3f(v, z),
							curve.GetNormal(distAlongCurve)));
					}
				}
				if (half == 0)
				{
					sideVertices[side].push_back(VertexPosNorm(
						curve.Middle(),
						curve.GetNormal(curve.horizontalCurve.first.length)));
				}
				dist += arc.length;
			}
			sideVertices[side].push_back(VertexPosNorm(
				curve.End(),
				curve.GetNormal(curve.Length())));
		}
	}
	ZipArcs(outVertices, outIndices, sideVertices[0], sideVertices[1]);
}

