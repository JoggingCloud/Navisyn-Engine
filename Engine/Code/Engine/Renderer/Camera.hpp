#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera
{
public:
	enum Mode
	{
		eMode_Orthographic,
		eMode_Perspective,
		eMode_Count
	};

	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight);
	Vec2 GetOrthographicBottomLeft() const;
	Vec2 GetOrthographicTopRight() const;
	AABB2 GetOrthographicView() const;

	void SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float zNear, float zFar);
	void SetPerspectiveView(float aspect, float fov, float zNear, float zFar);
	void SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	void SetTransform(const Vec3& position, const EulerAngles& orientation);
	void SetCenter(Vec2 const& center);
	void SetViewport(AABB2 const& cameraViewport);

	AABB2 GetViewport() const;
	//void Translate2D(Vec2 const& translation);
	Vec3 GetPosition() const;
	EulerAngles GetOrientation() const;
	Vec3 GetForwardVector() const;
	Vec3 GetRightVector() const;
	Vec3 GetUpVector() const;

	Mat44 GetModelMatrix() const;
	Mat44 GetViewMatrix() const;
	Mat44 GetRenderMatrix() const;
	Mat44 GetOrthographicMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;

public:
	AABB2 m_viewPort = AABB2::ZERO_TO_ONE;
	AABB2 m_orthographicView = AABB2::ZERO_TO_ONE;
	Vec2 m_orhographicBottomLeft = Vec2::ZERO;
	Vec2 m_orthographicTopRight = Vec2::ZERO;
	float m_orthographicNear = 0.f;
	float m_orthographicFar = 0.f;

	float m_perspectiveAspect = 0.f;
	float m_perspectiveFOV = 0.f;
	float m_perspectiveNear = 0.f;
	float m_perspectiveFar = 0.f;

public:
	Mode m_mode = eMode_Orthographic;

	Vec3 m_position;
	EulerAngles m_orientation;

	Vec3 m_renderIBasis = Vec3(1.f, 0.f, 0.f);
	Vec3 m_renderJBasis = Vec3(0.f, 1.f, 0.f);
	Vec3 m_renderKBasis = Vec3(0.f, 0.f, 1.f);
};