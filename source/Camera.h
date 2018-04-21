#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <cmgMath/cmg_math.h>


struct CameraState
{
	Vector2f m_position;
	float m_viewHeight;
	float m_rotation;
	float m_aspectRatio;

	Vector2f GetLeft() const
	{
		Vector2f left = -Vector2f::UNITX;
		left = left.Rotate(-m_rotation);
		return left;
	}

	Vector2f GetUp() const
	{
		Vector2f up = -Vector2f::UNITY;
		up = up.Rotate(-m_rotation);
		return up;
	}

	Rect2f GetRect()
	{
		Rect2f rect(m_position, Vector2f::ZERO);
		rect.Inflate(m_viewHeight * 0.5f * m_aspectRatio, m_viewHeight * 0.5f);
		return rect;
	}

	Vector2f ToWorldSpace(const Vector2f& screenPos, const Vector2f& screenSize)
	{
		Vector2f v = (((screenPos / screenSize) * 2.0f) - Vector2f::ONE) * Vector2f(1, -1);

		return ToWorldSpace(v);
	}

	Vector2f ToWorldSpace(const Vector2f& cameraPos)
	{
		Matrix4f matrix = GetCameraToWorldMatrix();
		return (matrix * Vector4f(cameraPos, 1.0f, 1.0f)).GetXY();
	}

	Vector2f ToCameraSpace(const Vector2f& worldPos)
	{
		Matrix4f matrix = GetWorldToCameraMatrix();
		return (matrix * Vector4f(worldPos, 1.0f, 1.0f)).GetXY();
	}

	Vector2f ToScreenSpace(const Vector2f& worldPos, const Vector2f& screenSize)
	{
		Matrix4f matrix = GetWorldToCameraMatrix();
		Vector2f v = (matrix * Vector4f(worldPos, 1.0f, 1.0f)).GetXY();
		v = (((v * Vector2f(1, -1)) + Vector2f::ONE) * 0.5f) * screenSize;
		return v;
	}

	Matrix4f GetWorldToCameraMatrix()
	{
		// World Sapce --> Camera Space
		return Matrix4f::CreateScale(1.0f / (m_viewHeight * m_aspectRatio * 0.5f), -1.0f / (m_viewHeight * 0.5f), 1.0f) *
			Matrix4f::CreateRotation(Vector3f::FORWARD, -m_rotation) *
			Matrix4f::CreateTranslation(-m_position.x, -m_position.y, 0.0f);
	}
	
	Matrix4f GetCameraToWorldMatrix()
	{
		// Camera Space --> World Space
		return Matrix4f::CreateTranslation(m_position.x, m_position.y, 0.0f) *
			Matrix4f::CreateRotation(Vector3f::FORWARD, m_rotation) *
			Matrix4f::CreateScale(m_viewHeight * m_aspectRatio * 0.5f, -m_viewHeight * 0.5f, 1.0f);
	}

	static CameraState Lerp(const CameraState& a, const CameraState& b, float t)
	{
		CameraState result;
		result.m_position = Vector2f::Lerp(a.m_position, b.m_position, t);
		result.m_aspectRatio = a.m_aspectRatio;
		result.m_viewHeight = Math::Lerp(a.m_viewHeight, b.m_viewHeight, t);
		result.m_rotation = Math::Lerp(a.m_rotation, b.m_rotation, t);
		return result;
	}
};


#endif // _CAMERA_H_