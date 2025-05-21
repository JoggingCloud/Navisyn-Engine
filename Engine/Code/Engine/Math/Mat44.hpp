#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec4.hpp"

struct Mat44
{
	enum { Ix, Iy, Iz, Iw, Jx, Jy, Jz, Jw, Kx, Ky, Kz, Kw, Tx, Ty, Tz, Tw }; // index nicknames, [0] through [15]
	float m_values[16]; // stored in "basis major" order (Ix, Iy, Iz, Iw, Jx...) - translation in [12,13,14]

	Mat44(); // Default constructor is identity matrix. NOT all zeros!
	explicit Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D);
	explicit Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D);
	explicit Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D);
	explicit Mat44(float const* sixteenValuesBasisMajor);

	static Mat44 const CreateTranslation2D(Vec2 const& translationXY);
	static Mat44 const CreateTranslation3D(Vec3 const& translationXYZ);
	static Mat44 const CreateUniformScale2D(float uniformScaleXY);
	static Mat44 const CreateUniformScale3D(float uniformScaleXYZ);
	static Mat44 const CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY);
	static Mat44 const CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ);

	static Mat44 const CreateZRotationDegrees(float rotationDegreesAboutZ);
	static Mat44 const CreateYRotationDegrees(float rotationDegreesAboutY);
	static Mat44 const CreateXRotationDegrees(float rotationDegreesAboutX);
	static Mat44 const CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	static Mat44 const CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar);

	Vec2 const TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const; // Assumes z = 0, w = 0
	Vec3 const TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const; // Assumes w = 0
	Vec2 const TransformPosition2D(Vec2 const& positionXY) const; // Assumes z = 0, w = 0
	Vec3 const TransformPosition3D(Vec3 const& position3D) const; // Assumes w = 1
	Vec4 const TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const; // w is provided

	float* GetAsFloatArray(); // Non-const (mutable) version
	float const* GetAsFloatArray() const; // const version, used only when Mat44 is const
	Vec2 const GetIBasis2D() const;
	Vec2 const GetJBasis2D() const;
	Vec2 const GetTranslation2D() const;
	Vec3 const GetIBasis3D() const;
	Vec3 const GetJBasis3D() const;
	Vec3 const GetKBasis3D() const;
	Vec3 const GetTranslation3D() const;
	Vec4 const GetIBasis4D() const;
	Vec4 const GetJBasis4D() const;
	Vec4 const GetKBasis4D() const;
	Vec4 const GetTranslation4D() const;
	Mat44 const GetOrthonormalInverse() const; // Only works for orthonormal affine matrices

	void SetTranslation2D(Vec2 const& translationXY); // Sets translationZ = 0, translationW = 1
	void SetTranslation3D(Vec3 const& translationXYZ); // Sets translationW = 1
	void SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D); // Sets z = 0, w = 0 for i & j; does not modify k or t
	void SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY); // Sets z = 0, w = 0 for i,j,t; Does not modify k
	
	void SetI3D(Vec3 const& iBasis3D);
	void SetJ3D(Vec3 const& jBasis3D);
	void SetK3D(Vec3 const& kBasis3D);

	void SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D); // Sets w = 0 for i,j,k
	void SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ); // Sets w = 0 for i,j,k, w = 1 for t
	void SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D);
	void SetScale(Vec3 const& scaleFactors);
	void Transpose(); // Swaps columns with rows 
	void Orthonormalize_IFwd_JLeft_KUp(); // Forward is canonical, Up is secondary, left tertiary

	void Append(Mat44 const& appendThis); // multiply on right in column notation / on left in row notation
	void AppendZRotation(float degreesRotationAboutZ); // Same as appending (*= in column notation) a Z-rotation matrix
	void AppendYRotation(float degreesRotationAboutY); // Same as appending (*= in column notation) a Y-rotation matrix
	void AppendXRotation(float degreesRotationAboutX); // Same as appending (*= in column notation) a X-rotation matrix
	void AppendTranslation2D(Vec2 const& translationXY); // Same as appending (*= in column notation) a X-rotation matrix
	void AppendTranslation3D(Vec3 const& translationXYZ); // Same as appending (*= in column notation) a translation matrix
	void AppendScaleUniform2D(float uniformScaleXY); // K and T bases should remain unaffected
	void AppendScaleUniform3D(float uniformScaleXYZ); // Translation should remain unaffected
	void AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY); // K and T bases should remain unaffected
	void AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ); // Translation should remain unaffected

	bool IsIdentity() const;

	bool operator !=(const Mat44& other) const;
};