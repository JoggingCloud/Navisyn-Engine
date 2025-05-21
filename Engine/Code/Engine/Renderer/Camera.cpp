#include <windows.h>
#include "Engine/Renderer/Camera.hpp"

void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight)
{
	SetOrthographicView(bottomLeft, topRight, 0.f, 1.f);
}

AABB2 Camera::GetOrthographicView() const
{
	return m_orthographicView;
}

void Camera::SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float zNear, float zFar)
{
	m_mode = Camera::eMode_Orthographic;
	m_orhographicBottomLeft = bottomLeft;
	m_orthographicTopRight = topRight;
	m_orthographicNear = zNear;
	m_orthographicFar = zFar;

	m_orthographicView = AABB2(bottomLeft, topRight);
}

void Camera::SetPerspectiveView(float aspect, float fov, float zNear, float zFar)
{
	m_mode = Camera::eMode_Perspective;
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = zNear;
	m_perspectiveFar = zFar;
}

void Camera::SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

void Camera::SetTransform(const Vec3& position, const EulerAngles& orientation)
{
	m_position = position;
	m_orientation = orientation;
}

void Camera::SetCenter(Vec2 const& center)
{
	m_orthographicView.SetCenter(center);
}

void Camera::SetViewport(AABB2 const& cameraViewport)
{
	m_viewPort.m_mins.x = cameraViewport.m_mins.x;
	m_viewPort.m_maxs.x = cameraViewport.m_maxs.x;

	m_viewPort.m_mins.y = 1.f - cameraViewport.m_maxs.y;
	m_viewPort.m_maxs.y = 1.f - cameraViewport.m_mins.y;
}

AABB2 Camera::GetViewport() const
{
	return m_viewPort;
}

Vec2 Camera::GetOrthographicBottomLeft() const
{
	return m_orhographicBottomLeft;
}

Vec2 Camera::GetOrthographicTopRight() const
{
	return m_orthographicTopRight;
}

Vec3 Camera::GetPosition() const
{
	return m_position;
}

EulerAngles Camera::GetOrientation() const
{
	return m_orientation;
}

Vec3 Camera::GetForwardVector() const
{
	return m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
}

Vec3 Camera::GetRightVector() const
{
	return -m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D();
}

Vec3 Camera::GetUpVector() const
{
	return m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetKBasis3D();
}

Mat44 Camera::GetViewMatrix() const
{
	Mat44 translation = Mat44::CreateTranslation3D(m_position);
	Mat44 orientation = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translation.Append(orientation);
	return translation.GetOrthonormalInverse();
}

Mat44 Camera::GetRenderMatrix() const
{
	// Create a matrix using the render basis vectors
	Mat44 renderMatrix(m_renderIBasis, m_renderJBasis, m_renderKBasis, Vec3::ZERO);
	return renderMatrix;
}

Mat44 Camera::GetOrthographicMatrix() const
{
	return Mat44::CreateOrthoProjection(m_orhographicBottomLeft.x, m_orthographicTopRight.x, m_orhographicBottomLeft.y, m_orthographicTopRight.y, m_orthographicNear, m_orthographicFar);
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
}

Mat44 Camera::GetProjectionMatrix() const
{
	if (m_mode == Camera::eMode_Orthographic)
	{
		Mat44 projectionMatrix = GetOrthographicMatrix();
		projectionMatrix.Append(GetRenderMatrix());
		return projectionMatrix;
	}
	else
	{
		Mat44 projectionMatrix = GetPerspectiveMatrix();
		projectionMatrix.Append(GetRenderMatrix());
		return projectionMatrix ;
	}
}

Mat44 Camera::GetModelMatrix() const
{
	Mat44 translation = Mat44::CreateTranslation3D(m_position);
	Mat44 orientation = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();

	translation.Append(orientation);
	return translation;
}
