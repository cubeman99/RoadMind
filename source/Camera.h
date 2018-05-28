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



class Camera
{
public:
	// Constructors

	Camera();

	// Getters

	Vector3f GetPosition() const;
	Quaternion GetOrientation() const;
	const Matrix4f& GetViewProjectionMatrix() const;
	const Matrix4f& GetViewMatrix() const;
	const Matrix4f& GetInverseViewMatrix() const;
	const Matrix4f& GetProjectionMatrix() const;
	Ray GetRay(const Vector2f& screenCoordinates) const;
	bool IsInsideView(const Vector3f& worldPoint) const;

	// Setters

	void SetPerspective(float aspectRatio, float fieldOfViewY,
		float minDistance, float maxDistance);
	void SetAspectRatio(float aspectRatio);
	void SetFieldOfView(float fieldOfViewY);
	void SetPosition(const Vector3f& position);
	void SetOrientation(const Quaternion& orientation);

private:
	// Internal Methods

	void CalcProjectionMatrix();
	void CalcViewMatrix();

private:
	float m_fieldOfView;
	float m_aspectRatio;
	float m_minDistance;
	float m_maxDistance;

	Vector3f m_position;
	Quaternion m_orientation;
	Matrix4f m_viewMatrix;
	Matrix4f m_viewMatrixInv;
	Matrix4f m_projectionMatrix;
	Matrix4f m_viewProjectionMatrix;
};


#endif // _CAMERA_H_