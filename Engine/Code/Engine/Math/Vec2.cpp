#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include <math.h>
#include <chrono>

Vec2 const Vec2::ZERO = Vec2(0.f,0.f);

//-----------------------------------------------------------------------------------------------
Vec2::Vec2(const Vec2& copy)
	: x(copy.x) 
	, y(copy.y)
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	Vec2 temp;
	temp.x = length * CosDegrees(ConvertRadiansToDegrees(orientationRadians));
	temp.y = length * SinDegrees(ConvertRadiansToDegrees(orientationRadians));
	return temp;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( vecToAdd.x + x, vecToAdd.y + y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2(  x * uniformScale, y * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2(vecToMultiply.x, vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2(x / inverseScale, y / inverseScale);
}


bool Vec2::operator<(const Vec2& compare) const
{
	if (x != compare.x)
	{
		return x < compare.x;
	}
	return y < compare.y;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( uniformScale * vecToScale.x, uniformScale * vecToScale.y);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return compare.x == x && compare.y == y;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return compare.x != x || compare.y != y;
}

Vec2 const Vec2::GetReflected(Vec2 const& impactSurfaceNormal) const
{
	// Calculate the dot product of the vector and the impact surface normal
	float dotProduct = DotProduct2D(*this, impactSurfaceNormal);

	return *this - (2.f * dotProduct * impactSurfaceNormal);
}

// Const Methods
float Vec2::GetLength() const
{
	return sqrtf((x * x) + (y * y));
}
float Vec2::GetLengthSquared() const
{
	return ((x * x) + (y * y));
}

float Vec2::GetOrientationRadians() const
{
	return atan2f(y, x);
}
float Vec2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees(atan2f(y, x));
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2(-y, x);
}
Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y,-x);
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	Vec2 rotatedVector;
	rotatedVector.x = x;
	rotatedVector.y = y;
	rotatedVector.RotateRadians(deltaRadians);
	return rotatedVector;
}
Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	Vec2 rotatedVector;
	rotatedVector.x = x;
	rotatedVector.y = y;
	rotatedVector.RotateDegrees(deltaDegrees);
	return rotatedVector;
}

Vec2 const Vec2::GetClamped(float maxLength) const
{
	Vec2 returnVector;
	returnVector.x = x;
	returnVector.y = y;
 	float length = GetLength();
 	if (length > maxLength)
 	{
		float scale = maxLength / length;
		returnVector *= scale;
 	}
	return returnVector;
}
Vec2 const Vec2::GetNormalized() const
{
	float xDistance = 1 * cosf(atan2f(static_cast<float>(y), static_cast<float>(x)));
	float yDistance = 1 * sinf(atan2f(static_cast<float>(y), static_cast<float>(x)));
	
	return Vec2(xDistance, yDistance);
}

//Static methods (e.g. creation functions)
Vec2 const MakeFromPolarRadians(float orientationRadians, float length)
{
	float x;
	float y;
	x = length * cosf(orientationRadians);
	y = length * sinf(orientationRadians);
	return Vec2(x,y);
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float x;
	float y;
	x = length * CosDegrees(orientationDegrees);
	y = length * SinDegrees(orientationDegrees);
	return Vec2(x,y);
}

// Non Const Methods 
void Vec2::SetOrientationDegrees(float orientationDegrees)
{
	float length = GetLength();
	x = length * CosDegrees(orientationDegrees);
	y = length * SinDegrees(orientationDegrees);
}
void Vec2::SetOrientationRadians(float orientationRadians)
{
	float length = GetLength();
	x = length * cosf(orientationRadians);
	y = length * sinf(orientationRadians);
}

void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	x = newLength * CosDegrees(newOrientationDegrees);
	y = newLength * SinDegrees(newOrientationDegrees);
}
void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
}

void Vec2::Rotate90Degrees()
{
	Vec2 rotated = GetRotated90Degrees();
	x = rotated.x;
	y = rotated.y;
}
void Vec2::RotateMinus90Degrees()
{
	Vec2 negativeRotated = GetRotatedMinus90Degrees();
	x = negativeRotated.x;
	y = negativeRotated.y;
}

void Vec2::RotateRadians(float deltaRadians)
{
	float length = GetLength();
	float thetaRadians;
	thetaRadians = atan2f(y, x);
	thetaRadians += deltaRadians;
	x = length * cosf(thetaRadians);
	y = length * sinf(thetaRadians);
}
void Vec2::RotateDegrees(float deltaDegrees)
{
	float length = GetLength();
	float thetaDegrees;
	thetaDegrees = Atan2Degrees(y, x);
	thetaDegrees += deltaDegrees;
	x = length * CosDegrees(thetaDegrees);
	y = length * SinDegrees(thetaDegrees);
}

void Vec2::SetLength(float newLength)
{
	this->Normalize();
	x *= newLength;
	y *= newLength;
}
void Vec2::ClampLength(float maxLength)
{
	float length = GetLength();
	if (length > maxLength)
	{
		float scale = maxLength / length;
		x *= scale;
		y *= scale;
	}
}

void Vec2::Normalize()
{
	float length = GetLength();
	if (length > 0.f)
	{
		float scale = 1.f / length;
		x *= scale;
		y *= scale;
	}
}
float Vec2::NormalizeAndGetPreviousLength()
{
	float length = GetLength();
	if (length > 0.f)
	{
		float scale = 1.f / length;
		x *= scale;
		y *= scale;
	}
	return length;
}

void Vec2::Reflect(Vec2 const& impactSurfaceNormal)
{
	// Calculate the dot product of the vector and the impact surface normal
	float dotProduct = DotProduct2D(*this, impactSurfaceNormal);

	// Update the vector to represent its reflection. 
	// Multiply by 2 to ensure the angle of incidence is equal to the angle of reflection when light reflects of a surface.
	// So adjust by multiplying the dot product by 2 to get it to reflect
	*this -= (2.f * dotProduct * impactSurfaceNormal);
}

void Vec2::SetFromText(char const* text)
{
	Strings components = SplitStringOnDelimiter(text, ',');

	if (components.size() == 2)
	{
		x = static_cast<float>(std::stof(components[0].c_str()));
		y = static_cast<float>(std::stof(components[1].c_str()));
	}
	else
	{
		x = y = 0.0f;
	}
}

Vec2 Vec2::GetRandomDirection2D()
{
	RandomNumberGenerator rng;
	rng.SetSeed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	float angle = rng.SRollRandomFloatInRange(0.f, 360.f);
	return Vec2::MakeFromPolarDegrees(angle);
}

Vec2 Vec2::Lerp(const Vec2& other, float t) const
{
	return Vec2{ x + (other.x - x) * t, y + (other.y - y) * t };
}
