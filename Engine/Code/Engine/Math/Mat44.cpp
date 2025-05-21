#include "Engine/Math/Mat44.hpp"

Mat44::Mat44()
{
	for (int valueIndex = 0; valueIndex < 16; valueIndex++)
	{
		m_values[valueIndex] = (valueIndex % 5 == 0) ? 1.0f : 0.0f;
	}
}

Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iw] = 0.0f;
	m_values[Iz] = 0.0f;
	
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jw] = 0.0f;
	m_values[Jz] = 0.0f;

	m_values[Kx] = 0.f;
	m_values[Ky] = 0.f;
 	m_values[Kz] = 1.f;
	m_values[Kw] = 0.f;

	// Set Translation
	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	
	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	
	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;

	// 4th basis vector
	m_values[Iw] = 0.0f;
	m_values[Jw] = 0.0f;
	m_values[Kw] = 0.0f;
	m_values[Tw] = 1.0f;

	// Set translation
	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
}

Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;
	
	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;
	
	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	// Set translation
	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	for (int valueindex = 0; valueindex < 16; valueindex++)
	{
		m_values[valueindex] = sixteenValuesBasisMajor[valueindex];
	}
}

Mat44 const Mat44::CreateTranslation2D(Vec2 const& translationXY)
{
	Mat44 translationMatrix;
	
	translationMatrix.m_values[Tx] = translationXY.x;
	translationMatrix.m_values[Ty] = translationXY.y;

	return translationMatrix;
}

Mat44 const Mat44::CreateTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 translationMatrix;
	
	translationMatrix.m_values[Tx] = translationXYZ.x;
	translationMatrix.m_values[Ty] = translationXYZ.y;
	translationMatrix.m_values[Tz] = translationXYZ.z;

	return translationMatrix;
}

Mat44 const Mat44::CreateUniformScale2D(float uniformScaleXY)
{
	Mat44 scaleMatrix;
	
	scaleMatrix.m_values[Ix] = uniformScaleXY;
	scaleMatrix.m_values[Jy] = uniformScaleXY;

	return scaleMatrix;
}

Mat44 const Mat44::CreateUniformScale3D(float uniformScaleXYZ)
{
	Mat44 scaleMatrix;

	scaleMatrix.m_values[Ix] = uniformScaleXYZ;
	scaleMatrix.m_values[Jy] = uniformScaleXYZ;
	scaleMatrix.m_values[Kz] = uniformScaleXYZ;

	return scaleMatrix;
}

Mat44 const Mat44::CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 scaleMatrix;

	scaleMatrix.m_values[Ix] = nonUniformScaleXY.x;
	scaleMatrix.m_values[Jy] = nonUniformScaleXY.y;

	return scaleMatrix;
}

Mat44 const Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 scaleMatrix;

	scaleMatrix.m_values[Ix] = nonUniformScaleXYZ.x;
	scaleMatrix.m_values[Jy] = nonUniformScaleXYZ.y;
	scaleMatrix.m_values[Kz] = nonUniformScaleXYZ.z;

	return scaleMatrix;
}

Mat44 const Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	Mat44 rotationMatrix;

	float cosTheta = CosDegrees(rotationDegreesAboutZ);
	float sinTheta = SinDegrees(rotationDegreesAboutZ);

	rotationMatrix.m_values[Ix] = cosTheta;
	rotationMatrix.m_values[Iy] = sinTheta;
	rotationMatrix.m_values[Jx] = -sinTheta;
	rotationMatrix.m_values[Jy] = cosTheta;

	return rotationMatrix;
}

Mat44 const Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	Mat44 rotationMatrix;

	float cosTheta = CosDegrees(rotationDegreesAboutY);
	float sinTheta = SinDegrees(rotationDegreesAboutY);

	rotationMatrix.m_values[Ix] = cosTheta;
	rotationMatrix.m_values[Iz] = -sinTheta;
	rotationMatrix.m_values[Kx] = sinTheta;
	rotationMatrix.m_values[Kz] = cosTheta;

	return rotationMatrix;
}

Mat44 const Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	Mat44 rotationMatrix;

	float cosTheta = CosDegrees(rotationDegreesAboutX);
	float sinTheta = SinDegrees(rotationDegreesAboutX);

	rotationMatrix.m_values[Jy] = cosTheta;
	rotationMatrix.m_values[Jz] = sinTheta;
	rotationMatrix.m_values[Ky] = -sinTheta;
	rotationMatrix.m_values[Kz] = cosTheta;

	return rotationMatrix;
}

Mat44 const Mat44::CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Mat44 projection;

	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	float tz = -zNear / (zFar - zNear);

	projection.m_values[Ix] = 2.f / (right - left);
	projection.m_values[Jy] = 2.f / (top - bottom);
	projection.m_values[Kz] = 1.f / (zFar - zNear);
	projection.m_values[Tx] = tx;
	projection.m_values[Ty] = ty;
	projection.m_values[Tz] = tz;
	projection.m_values[Tw] = 1.f;

	return projection;
}

Mat44 const Mat44::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat44 projection;

	// Convert fovYDegrees to radians
	float fovYRadians = ConvertDegreesToRadians(fovYDegrees);

	// Calculate the top coordinate of the view frustum
	float topCoord = tanf(fovYRadians / 2.0f) * zNear;

	// Calculate the bottom, left, and right coordinates of the view frustum
	float bottomCoord = -topCoord;
	float leftCoord = -topCoord * aspect;
	float rightCoord = topCoord * aspect;

	if (zNear == 0.0f)
	{
		projection.m_values[Ix] = 1.f;
		projection.m_values[Jy] = 1.f;
	}
	else
	{
		projection.m_values[Ix] = (2.0f * zNear) / (rightCoord - leftCoord);
		projection.m_values[Jy] = (2.0f * zNear) / (topCoord - bottomCoord);
	}

	// Construct the perspective projection matrix
	projection.m_values[Kz] = (zFar) / (zFar - zNear);
	projection.m_values[Kw] = 1.0f;
	projection.m_values[Tz] = -(zFar * zNear) / (zFar - zNear);
	projection.m_values[Tw] = 0.0f;

	return projection;
}

Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
	Vec2 result;
	
	result.x = (vectorQuantityXY.x * m_values[Ix]) + (vectorQuantityXY.y * m_values[Jx]);
	result.y = (vectorQuantityXY.x * m_values[Iy]) + (vectorQuantityXY.y * m_values[Jy]);

	return result;
}

Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	Vec3 result;

	result.x = (vectorQuantityXYZ.x * m_values[Ix]) + (vectorQuantityXYZ.y * m_values[Jx]) + (vectorQuantityXYZ.z * m_values[Kx]);
	result.y = (vectorQuantityXYZ.x * m_values[Iy]) + (vectorQuantityXYZ.y * m_values[Jy]) + (vectorQuantityXYZ.z * m_values[Ky]);
	result.z = (vectorQuantityXYZ.x * m_values[Iz]) + (vectorQuantityXYZ.y * m_values[Jz]) + (vectorQuantityXYZ.z * m_values[Kz]);

	return result;
}

Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	Vec2 result;

	result.x = (positionXY.x * m_values[Ix]) + (positionXY.y * m_values[Jx]) + m_values[Tx];
	result.y = (positionXY.x * m_values[Iy]) + (positionXY.y * m_values[Jy]) + m_values[Ty];

	return result;
}

Vec3 const Mat44::TransformPosition3D(Vec3 const& position3D) const
{
// 	Vec3 result;
// 
// 	Vec4 ax = Vec4(m_values[Ix], m_values[Jx], m_values[Kx], m_values[Tx]);
// 	Vec4 ay = Vec4(m_values[Iy], m_values[Jy], m_values[Ky], m_values[Ty]);
// 	Vec4 az = Vec4(m_values[Iz], m_values[Jz], m_values[Kz], m_values[Tz]);
// 
// 	Vec4 vecPos = Vec4(position3D.x, position3D.y, position3D.z, 1.0f);
// 
// 	result.x = DotProduct4D(ax, vecPos);
// 	result.y = DotProduct4D(ay, vecPos);
// 	result.z = DotProduct4D(az, vecPos);
// 
// 	return result;

	Vec4 aX = Vec4(m_values[Ix], m_values[Jx], m_values[Kx], m_values[Tx]);
	Vec4 aY = Vec4(m_values[Iy], m_values[Jy], m_values[Ky], m_values[Ty]);
	Vec4 aZ = Vec4(m_values[Iz], m_values[Jz], m_values[Kz], m_values[Tz]);

	Vec4 vectorQuantity = Vec4(position3D.x, position3D.y, position3D.z, 1.0f);

	return Vec3(DotProduct4D(aX, vectorQuantity), DotProduct4D(aY, vectorQuantity), DotProduct4D(aZ, vectorQuantity));
}

Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	Vec4 result;

	result.x = (homogeneousPoint3D.x * m_values[Ix]) + (homogeneousPoint3D.y * m_values[Jx]) + (homogeneousPoint3D.z * m_values[Kx]) + (homogeneousPoint3D.w * m_values[Tx]);
	result.y = (homogeneousPoint3D.x * m_values[Iy]) + (homogeneousPoint3D.y * m_values[Jy]) + (homogeneousPoint3D.z * m_values[Ky]) + (homogeneousPoint3D.w * m_values[Ty]);
	result.z = (homogeneousPoint3D.x * m_values[Iz]) + (homogeneousPoint3D.y * m_values[Jz]) + (homogeneousPoint3D.z * m_values[Kz]) + (homogeneousPoint3D.w * m_values[Tz]);
	result.w = (homogeneousPoint3D.x * m_values[Iw]) + (homogeneousPoint3D.y * m_values[Jw]) + (homogeneousPoint3D.z * m_values[Kw]) + (homogeneousPoint3D.w * m_values[Tw]);

	return result;
}

float* Mat44::GetAsFloatArray()
{
	return m_values;
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 orthonormalInverse;
	
	// Get i, j, k basis3D
	Vec3 iBasis = GetIBasis3D();
	Vec3 jBasis = GetJBasis3D();
	Vec3 kBasis = GetKBasis3D();

	// Rotation matrix
	Mat44 rotationMatrix;
	rotationMatrix.SetIJK3D(iBasis, jBasis, kBasis);

	// Translation matrix
	Vec3 translation = GetTranslation3D();
	translation = -translation;

	Mat44 translationMatrix;
	translationMatrix.SetTranslation3D(translation);

	rotationMatrix.Transpose();

	orthonormalInverse = rotationMatrix;
	orthonormalInverse.Append(translationMatrix);

	return orthonormalInverse;
}

void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;
}

void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;
	
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;
	
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetI3D(Vec3 const& iBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;
}

void Mat44::SetJ3D(Vec3 const& jBasis3D)
{
	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;
}

void Mat44::SetK3D(Vec3 const& kBasis3D)
{
	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;
}

void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;
}

void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;
	
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;
	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;
	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;
	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

void Mat44::SetScale(Vec3 const& scaleFactors)
{
	m_values[Ix] *= scaleFactors.x;
	m_values[Iy] *= scaleFactors.x;
	m_values[Iz] *= scaleFactors.x;

	m_values[Jx] *= scaleFactors.y;
	m_values[Jy] *= scaleFactors.y;
	m_values[Jz] *= scaleFactors.y;

	m_values[Kx] *= scaleFactors.z;
	m_values[Ky] *= scaleFactors.z;
	m_values[Kz] *= scaleFactors.z;
}

void Mat44::Transpose()
{
	Mat44 opposite(*this);

	m_values[Ix] = opposite.m_values[Ix];
	m_values[Iy] = opposite.m_values[Jx];
	m_values[Iz] = opposite.m_values[Kx];
	m_values[Iw] = opposite.m_values[Tx];
				   
	m_values[Jx] = opposite.m_values[Iy];
	m_values[Jy] = opposite.m_values[Jy];
	m_values[Jz] = opposite.m_values[Ky];
	m_values[Jw] = opposite.m_values[Ty];
				   
	m_values[Kx] = opposite.m_values[Iz];
	m_values[Ky] = opposite.m_values[Jz];
	m_values[Kz] = opposite.m_values[Kz];
	m_values[Kw] = opposite.m_values[Tz];
				   
	m_values[Tx] = opposite.m_values[Iw];
	m_values[Ty] = opposite.m_values[Jw];
	m_values[Tz] = opposite.m_values[Kw];
	m_values[Tw] = opposite.m_values[Tw];
}

void Mat44::Orthonormalize_IFwd_JLeft_KUp()
{
	Vec3 forwardI = GetIBasis3D();
	Vec3 leftJ = GetJBasis3D();
	Vec3 upK = GetKBasis3D();

	forwardI.Normalize();
	
	Vec3 upOnForwardProjection = DotProduct3D(upK, forwardI) * forwardI;
	upK -= upOnForwardProjection;
	upK.Normalize();

	Vec3 leftOnForwardProjection = DotProduct3D(leftJ, forwardI) * forwardI;
	leftJ -= leftOnForwardProjection;

	Vec3 leftOnUpProjection = DotProduct3D(leftJ, upK) * upK;
	leftJ -= leftOnUpProjection;
	leftJ.Normalize();

	SetIJK3D(forwardI, leftJ, upK);
}

void Mat44::Append(Mat44 const& appendThis)
{
	Mat44 copyOfThis = *this; // Make a copy of my old values, so we don't have to pollute them as we go
	float const* priorMatrix = copyOfThis.m_values; // column-notation nickname for the prior matrix, for brevity 
	float const* newMatrix = appendThis.m_values; // column-notation nickname for the new matrix, for brevity

	m_values[Ix] = (priorMatrix[Ix] * newMatrix[Ix]) + (priorMatrix[Jx] * newMatrix[Iy]) + (priorMatrix[Kx] * newMatrix[Iz]) + (priorMatrix[Tx] * newMatrix[Iw]);
	m_values[Iy] = (priorMatrix[Iy] * newMatrix[Ix]) + (priorMatrix[Jy] * newMatrix[Iy]) + (priorMatrix[Ky] * newMatrix[Iz]) + (priorMatrix[Ty] * newMatrix[Iw]);
	m_values[Iz] = (priorMatrix[Iz] * newMatrix[Ix]) + (priorMatrix[Jz] * newMatrix[Iy]) + (priorMatrix[Kz] * newMatrix[Iz]) + (priorMatrix[Tz] * newMatrix[Iw]);
	m_values[Iw] = (priorMatrix[Iw] * newMatrix[Ix]) + (priorMatrix[Jw] * newMatrix[Iy]) + (priorMatrix[Kw] * newMatrix[Iz]) + (priorMatrix[Tw] * newMatrix[Iw]);

	m_values[Jx] = (priorMatrix[Ix] * newMatrix[Jx]) + (priorMatrix[Jx] * newMatrix[Jy]) + (priorMatrix[Kx] * newMatrix[Jz]) + (priorMatrix[Tx] * newMatrix[Jw]);
	m_values[Jy] = (priorMatrix[Iy] * newMatrix[Jx]) + (priorMatrix[Jy] * newMatrix[Jy]) + (priorMatrix[Ky] * newMatrix[Jz]) + (priorMatrix[Ty] * newMatrix[Jw]);
	m_values[Jz] = (priorMatrix[Iz] * newMatrix[Jx]) + (priorMatrix[Jz] * newMatrix[Jy]) + (priorMatrix[Kz] * newMatrix[Jz]) + (priorMatrix[Tz] * newMatrix[Jw]);
	m_values[Jw] = (priorMatrix[Iw] * newMatrix[Jx]) + (priorMatrix[Jw] * newMatrix[Jy]) + (priorMatrix[Kw] * newMatrix[Jz]) + (priorMatrix[Tw] * newMatrix[Jw]);

	m_values[Kx] = (priorMatrix[Ix] * newMatrix[Kx]) + (priorMatrix[Jx] * newMatrix[Ky]) + (priorMatrix[Kx] * newMatrix[Kz]) + (priorMatrix[Tx] * newMatrix[Kw]);
	m_values[Ky] = (priorMatrix[Iy] * newMatrix[Kx]) + (priorMatrix[Jy] * newMatrix[Ky]) + (priorMatrix[Ky] * newMatrix[Kz]) + (priorMatrix[Ty] * newMatrix[Kw]);
	m_values[Kz] = (priorMatrix[Iz] * newMatrix[Kx]) + (priorMatrix[Jz] * newMatrix[Ky]) + (priorMatrix[Kz] * newMatrix[Kz]) + (priorMatrix[Tz] * newMatrix[Kw]);
	m_values[Kw] = (priorMatrix[Iw] * newMatrix[Kx]) + (priorMatrix[Jw] * newMatrix[Ky]) + (priorMatrix[Kw] * newMatrix[Kz]) + (priorMatrix[Tw] * newMatrix[Kw]);

	m_values[Tx] = (priorMatrix[Ix] * newMatrix[Tx]) + (priorMatrix[Jx] * newMatrix[Ty]) + (priorMatrix[Kx] * newMatrix[Tz]) + (priorMatrix[Tx] * newMatrix[Tw]);
	m_values[Ty] = (priorMatrix[Iy] * newMatrix[Tx]) + (priorMatrix[Jy] * newMatrix[Ty]) + (priorMatrix[Ky] * newMatrix[Tz]) + (priorMatrix[Ty] * newMatrix[Tw]);
	m_values[Tz] = (priorMatrix[Iz] * newMatrix[Tx]) + (priorMatrix[Jz] * newMatrix[Ty]) + (priorMatrix[Kz] * newMatrix[Tz]) + (priorMatrix[Tz] * newMatrix[Tw]);
	m_values[Tw] = (priorMatrix[Iw] * newMatrix[Tx]) + (priorMatrix[Jw] * newMatrix[Ty]) + (priorMatrix[Kw] * newMatrix[Tz]) + (priorMatrix[Tw] * newMatrix[Tw]);
}

void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Mat44 rotationMatrix = CreateZRotationDegrees(degreesRotationAboutZ);
	Append(rotationMatrix);
}

void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Mat44 rotationMatrix = CreateYRotationDegrees(degreesRotationAboutY);
	Append(rotationMatrix);
}

void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Mat44 rotationMatrix = CreateXRotationDegrees(degreesRotationAboutX);
	Append(rotationMatrix);
}

void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Mat44 translationMatrix = CreateTranslation2D(translationXY);
	Append(translationMatrix);
}

void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 translationMatrix = CreateTranslation3D(translationXYZ);
	Append(translationMatrix);
}

void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Mat44 scaleMatrix = CreateUniformScale2D(uniformScaleXY);
	Append(scaleMatrix);
}

void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Mat44 scaleMatrix = CreateUniformScale3D(uniformScaleXYZ);
	Append(scaleMatrix);
}

void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 scaleMatrix = CreateNonUniformScale2D(nonUniformScaleXY);
	Append(scaleMatrix);
}

void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 scaleMatrix = CreateNonUniformScale3D(nonUniformScaleXYZ);
	Append(scaleMatrix);
}

bool Mat44::IsIdentity() const
{
	Mat44 identity;
	for (int i = 0; i < 16; i++)
	{
		if (m_values[i] != identity.m_values[i])
		{
			return false;
		}
	}
	return true;
}

bool Mat44::operator!=(const Mat44& other) const
{
	for (int i = 0; i < 16; i++)
	{
		if (m_values[i] != other.m_values[i])
		{
			return true;
		}
	}
	return false;
}
