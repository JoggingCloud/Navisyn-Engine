#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <math.h>

Vec3 const Vec3::INVALID_POSITION = Vec3(-999.f, -999.f, -999.f);
Vec3 const Vec3::ZERO = Vec3(0.f, 0.f, 0.f);
Vec3 const Vec3::NORTH = Vec3(0.f, 1.f, 0.f);
Vec3 const Vec3::SOUTH = Vec3(0.f, -1.f, 0.f);
Vec3 const Vec3::EAST = Vec3(1.f, 0.f, 0.f);
Vec3 const Vec3::WEST = Vec3(-1.f, 0.f, 0.f);
Vec3 const Vec3::SKY = Vec3(0.f, 0.f, 1.f);
Vec3 const Vec3::GROUND = Vec3(0.f, 0.f, -1.f);


Vec3::Vec3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

Vec3::Vec3(Vec2 copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

void Vec3::Normalize()
{
	float length = GetLength();
	if (length > 0.f)
	{
		float scale = 1.f / length;
		x *= scale;
		y *= scale;
		z *= scale;
	}
}

Vec3 const Vec3::GetNormalized() const
{
	float scale = 1.0f / GetLength();

	Vec3 normal;

	normal.x = x * scale;
	normal.y = y * scale;
	normal.z = z * scale;

	return normal;
}

void Vec3::SetFromText(char const* text)
{
	Strings components = SplitStringOnDelimiter(text, ',');

	if (components.size() == 3)
	{
		x = static_cast<float>(std::stof(components[0].c_str()));
		y = static_cast<float>(std::stof(components[1].c_str()));
		z = static_cast<float>(std::stof(components[2].c_str()));
	}
	else
	{
		x = y = z = 0.0f;
	}
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator+(const Vec3& vecToAdd) const
{
	return Vec3(vecToAdd.x + x, vecToAdd.y + y, vecToAdd.z + z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-(const Vec3& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


const Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

//-----------------------------------------------------------------------------------------------
bool Vec3::operator<(const Vec3& compare) const
{
	if (x != compare.x)
	{
		return x < compare.x;
	}
	if (y != compare.y)
	{
		return y < compare.y;
	}
	return z < compare.z;
}

bool Vec3::operator>(const Vec3& compare) const
{
	if (x != compare.x)
	{
		return x > compare.x;
	}
	if (y != compare.y)
	{
		return y > compare.y;
	}
	return z > compare.z;
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/(float inverseScale) const
{
	return Vec3(x / inverseScale, y / inverseScale, z / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=(const Vec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=(const Vec3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=(const Vec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*(float uniformScale, const Vec3& vecToScale)
{
	return Vec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==(const Vec3& compare) const
{
	return compare.x == x && compare.y == y && compare.z == z;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=(const Vec3& compare) const
{
	return compare.x != x || compare.y != y || compare.z != z;
}

float GetChebyshevDistance3D(Vec3 const& start, Vec3 const& end)
{
	Vec3 diff = (end - start).GetAbsolute();
	return std::max({ diff.x, diff.y, diff.z });
}

bool Vec3::IsValid() const
{
	return !std::isnan(x) && !std::isnan(y) && !std::isnan(z);
}

bool Vec3::IsNearlyZero() const
{
	constexpr float epsilon = 0.00001f;
	return fabsf(x) < epsilon && fabsf(y) < epsilon && fabsf(z) < epsilon;
}

// Accessors (const methods)
float Vec3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}
float Vec3::GetLengthXY() const
{
	return sqrtf((x * x) + (y * y));
}

float Vec3::GetLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z));
}
float Vec3::GetLengthXYSquared() const
{
	return ((x * x) + (y * y));
}

float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}
float Vec3::GetAngleAboutZDegrees() const
{
	return ConvertRadiansToDegrees(atan2f(y, x));
}

Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
	float length = GetLengthXY();
	float thetaRadians;
	thetaRadians = atan2f(y, x);
	thetaRadians += deltaRadians;

	Vec3 rotatedVector;
	rotatedVector.x = length * cosf(thetaRadians);
	rotatedVector.y = length * sinf(thetaRadians);
	rotatedVector.z = z;
	return rotatedVector;
}
Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float length = GetLengthXY();
	float thetaDegrees;
	thetaDegrees = Atan2Degrees(y, x);
	thetaDegrees += deltaDegrees;

	Vec3 rotatedVector;
	rotatedVector.x = length * CosDegrees(thetaDegrees);
	rotatedVector.y = length * SinDegrees(thetaDegrees);
	rotatedVector.z = z;
	return rotatedVector;
}

Vec3 const Vec3::GetRotatedAboutXDegrees(float deltaDegrees) const
{
	float length = GetLengthXY();
	float thetaDegrees;
	thetaDegrees = Atan2Degrees(z, y);
	thetaDegrees += deltaDegrees;

	Vec3 rotatedVector;
	rotatedVector.x = x; 
	rotatedVector.y = length * CosDegrees(thetaDegrees);
	rotatedVector.z = length * SinDegrees(thetaDegrees);
	return rotatedVector;
}

Vec3 const Vec3::GetClamped(float maxLength) const
{
	Vec3 returnVector;
	returnVector.x = x;
	returnVector.y = y;
	returnVector.z = z;
	float length = GetLengthXY();
	if (length > maxLength)
	{
		float scale = maxLength / length;
		returnVector *= scale;
	}
	return returnVector;
}

Vec3 Vec3::GetAbsolute() const
{
	return Vec3(std::fabsf(x), std::fabsf(y), std::fabsf(z));
}

Vec3 Vec3::Min(Vec3 const& a, Vec3 const& b)
{
	return Vec3(
		(a.x < b.x) ? a.x : b.x,
		(a.y < b.y) ? a.y : b.y,
		(a.z < b.z) ? a.z : b.z
	);
}

Vec3 Vec3::Max(Vec3 const& a, Vec3 const& b)
{
	return Vec3(
		(a.x > b.x) ? a.x : b.x,
		(a.y > b.y) ? a.y : b.y,
		(a.z > b.z) ? a.z : b.z
	);
}

Vec3 const Vec3::MakeFromPolarRadians(float latitudeRadians, float longitudeRadians, float length)
{	
	float x = length * cosf(latitudeRadians) * cosf(longitudeRadians);
	float y = length * cosf(latitudeRadians) * sinf(longitudeRadians);
	float z = length * sinf(latitudeRadians);
	
	return Vec3(x, y, z);
}

Vec3 const Vec3::MakeFromPolarDegrees(float latitudeDegrees, float longitudeDegrees, float length)
{
	
	float x = length * CosDegrees(latitudeDegrees) * CosDegrees(longitudeDegrees);
	float y = length * CosDegrees(latitudeDegrees) * SinDegrees(longitudeDegrees);
	float z = length * SinDegrees(latitudeDegrees);
	
	return Vec3(x, y, z);
}

std::size_t Vec3Hash::operator()(const Vec3& v) const noexcept
{
	return std::hash<float>()(v.x) ^ (std::hash<float>{}(v.y) << 1) ^ (std::hash<float>()(v.z) << 2);
}

std::size_t Vec3PairHash::operator()(const std::pair<Vec3, Vec3>& pair) const noexcept
{
	return Vec3Hash()(pair.first) ^ (Vec3Hash()(pair.second) << 1);
}

bool Vec3PairEqual::operator()(const std::pair<Vec3, Vec3>& leftHash, const std::pair<Vec3, Vec3>& rightHash) const noexcept
{
	return leftHash.first == rightHash.first && leftHash.second == rightHash.second;
}
