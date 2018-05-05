#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <cmgMath/cmg_math.h>


struct CameraState
{
	Vector2f m_position;
	float m_viewHeight;
	float m_rotation;
	float m_aspectRatio;


	CameraState();


	Vector2f GetLeft() const;

	Vector2f GetUp() const;

	Rect2f GetRect();

	Vector2f ToWorldSpace(const Vector2f& screenPos, const Vector2f& screenSize);

	Vector2f ToWorldSpace(const Vector2f& cameraPos);

	Vector2f ToCameraSpace(const Vector2f& worldPos);

	Vector2f ToScreenSpace(const Vector2f& worldPos, const Vector2f& screenSize);

	Matrix4f GetWorldToCameraMatrix();
	
	Matrix4f GetCameraToWorldMatrix();

	static CameraState Lerp(const CameraState& a, const CameraState& b, float t);
};


#endif // _CAMERA_H_