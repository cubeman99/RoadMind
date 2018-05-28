#include "Camera.h"


CameraState::CameraState()
	: m_position(Vector2f::ZERO)
	, m_viewHeight(1.0f)
	, m_rotation(0.0f)
	, m_aspectRatio(1.0f)
{
}

Vector2f CameraState::GetLeft() const
{
	Vector2f left = -Vector2f::UNITX;
	left = left.Rotate(-m_rotation);
	return left;
}

Vector2f CameraState::GetUp() const
{
	Vector2f up = -Vector2f::UNITY;
	up = up.Rotate(-m_rotation);
	return up;
}

Rect2f CameraState::GetRect()
{
	Rect2f rect(m_position, Vector2f::ZERO);
	rect.Inflate(m_viewHeight * 0.5f * m_aspectRatio, m_viewHeight * 0.5f);
	return rect;
}

Vector2f CameraState::ToWorldSpace(const Vector2f& screenPos, const Vector2f& screenSize)
{
	Vector2f v = (((screenPos / screenSize) * 2.0f) - Vector2f::ONE) * Vector2f(1, -1);

	return ToWorldSpace(v);
}

Vector2f CameraState::ToWorldSpace(const Vector2f& cameraPos)
{
	Matrix4f matrix = GetCameraToWorldMatrix();
	return (matrix * Vector4f(cameraPos, 1.0f, 1.0f)).GetXY();
}

Vector2f CameraState::ToCameraSpace(const Vector2f& worldPos)
{
	Matrix4f matrix = GetWorldToCameraMatrix();
	return (matrix * Vector4f(worldPos, 1.0f, 1.0f)).GetXY();
}

Vector2f CameraState::ToScreenSpace(const Vector2f& worldPos, const Vector2f& screenSize)
{
	Matrix4f matrix = GetWorldToCameraMatrix();
	Vector2f v = (matrix * Vector4f(worldPos, 1.0f, 1.0f)).GetXY();
	v = (((v * Vector2f(1, -1)) + Vector2f::ONE) * 0.5f) * screenSize;
	return v;
}

Matrix4f CameraState::GetWorldToCameraMatrix()
{
	// World Sapce --> Camera Space
	return Matrix4f::CreateScale(1.0f / (m_viewHeight * m_aspectRatio * 0.5f), -1.0f / (m_viewHeight * 0.5f), 1.0f) *
		Matrix4f::CreateRotation(Vector3f::FORWARD, -m_rotation) *
		Matrix4f::CreateTranslation(-m_position.x, -m_position.y, 0.0f);
}

Matrix4f CameraState::GetCameraToWorldMatrix()
{
	// Camera Space --> World Space
	return Matrix4f::CreateTranslation(m_position.x, m_position.y, 0.0f) *
		Matrix4f::CreateRotation(Vector3f::FORWARD, m_rotation) *
		Matrix4f::CreateScale(m_viewHeight * m_aspectRatio * 0.5f, -m_viewHeight * 0.5f, 1.0f);
}

CameraState CameraState::Lerp(const CameraState& a, const CameraState& b, float t)
{
	CameraState result;
	result.m_position = Vector2f::Lerp(a.m_position, b.m_position, t);
	result.m_aspectRatio = a.m_aspectRatio;
	result.m_viewHeight = Math::Lerp(a.m_viewHeight, b.m_viewHeight, t);
	result.m_rotation = Math::Lerp(a.m_rotation, b.m_rotation, t);
	return result;
}



//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Camera::Camera()
	: m_position(Vector3f::ZERO)
	, m_orientation(Quaternion::IDENTITY)
	, m_aspectRatio(1.0f)
	, m_fieldOfView(1.0f)
	, m_minDistance(0.1f)
	, m_maxDistance(100.0f)
	, m_viewMatrix(Matrix4f::IDENTITY)
	, m_viewMatrixInv(Matrix4f::IDENTITY)
	, m_projectionMatrix(Matrix4f::IDENTITY)
	, m_viewProjectionMatrix(Matrix4f::IDENTITY)
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

Vector3f Camera::GetPosition() const
{
	return m_position;
}

Quaternion Camera::GetOrientation() const
{
	return m_orientation;
}

const Matrix4f& Camera::GetViewProjectionMatrix() const
{
	return m_viewProjectionMatrix;
}

const Matrix4f& Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

const Matrix4f& Camera::GetInverseViewMatrix() const
{
	return m_viewMatrixInv;
}

const Matrix4f& Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

Ray Camera::GetRay(const Vector2f& screenCoordinates) const
{
	// Get the direction in view space
    Vector2f screenSpace = screenCoordinates;
	screenSpace.x *= m_aspectRatio;
    float viewRatio = Math::Tan(m_fieldOfView * 0.5f);
	Vector3f direction(screenSpace * viewRatio, -1.0f);

	// Rotate the direction into world space
	direction.Rotate(m_orientation);
	return Ray(m_position, direction);
}

bool Camera::IsInsideView(const Vector3f& worldPoint) const
{
	Vector3f screenPoint = m_viewProjectionMatrix * worldPoint;
	return (screenPoint.x >= -1.0f && screenPoint.x <= 1.0f &&
		screenPoint.y >= -1.0f && screenPoint.y <= 1.0f);
}


//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------

void Camera::SetPerspective(float aspectRatio, float fieldOfViewY,
	float minDistance, float maxDistance)
{
	m_aspectRatio = aspectRatio;
	m_fieldOfView = fieldOfViewY;
	m_minDistance = minDistance;
	m_maxDistance = maxDistance;
	CalcProjectionMatrix();
}

void Camera::SetAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
	CalcProjectionMatrix();
}

void Camera::SetFieldOfView(float fieldOfViewY)
{
	m_fieldOfView = fieldOfViewY;
	CalcProjectionMatrix();
}

void Camera::SetPosition(const Vector3f& position)
{
	m_position = position;
	CalcViewMatrix();
}

void Camera::SetOrientation(const Quaternion& orientation)
{
	m_orientation = orientation;
	CalcViewMatrix();
}


//-----------------------------------------------------------------------------
// Internal Methods
//-----------------------------------------------------------------------------

void Camera::CalcViewMatrix()
{
	m_viewMatrix = Matrix4f::CreateRotation(m_orientation.GetConjugate()) *
		Matrix4f::CreateTranslation(-m_position);
	m_viewMatrixInv = Matrix4f::CreateTranslation(m_position) *
		Matrix4f::CreateRotation(m_orientation);
		
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

void Camera::CalcProjectionMatrix()
{
	m_projectionMatrix = Matrix4f::CreatePerspective(m_fieldOfView,
		m_aspectRatio, m_minDistance, m_maxDistance);
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

