#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <math.h>

IntVec3 const IntVec3::ZERO = IntVec3(0, 0, 0);
IntVec3 const IntVec3::NORTH = IntVec3(0, 1, 0);
IntVec3 const IntVec3::SOUTH = IntVec3(0, -1, 0);
IntVec3 const IntVec3::EAST = IntVec3(1, 0, 0);
IntVec3 const IntVec3::WEST = IntVec3(-1, 0, 0);
IntVec3 const IntVec3::SKY = IntVec3(0, 0, 1);
IntVec3 const IntVec3::GROUND = IntVec3(0, 0, -1);

IntVec3::IntVec3()
{
}

IntVec3::~IntVec3()
{
}

IntVec3::IntVec3(const IntVec3& copyFrom)
	:x(copyFrom.x)
	,y(copyFrom.y)
	,z(copyFrom.z)
{
}

IntVec3::IntVec3(int initialX, int initialY, int initialZ)
	:x(initialX)
	,y(initialY)
	,z(initialZ)
{
}

// Const Methods 
float IntVec3::GetLength() const
{
	return sqrtf(static_cast<float>(x * x) + static_cast<float>(y * y) + static_cast<float>(z * z));
}

int IntVec3::GetTaxicabLength() const
{
	return abs(x) + abs(y) + abs(z);
}

int IntVec3::GetLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z));
}

float IntVec3::GetOrientationRadians() const
{
	return atan2f(sqrtf(static_cast<float>(x * x) + static_cast<float>(y * y)), static_cast<float>(z));
}

float IntVec3::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees(atan2f(sqrtf(static_cast<float>(x * x) + static_cast<float>(y * y)), static_cast<float>(z)));
}

IntVec3 const IntVec3::GetRotated90Degrees() const
{
	return IntVec3(-y, x, z);
}

IntVec3 const IntVec3::GetRotatedMinus90Degrees() const
{
	return IntVec3(y,-x, z);
}

// Non-const Methods
void IntVec3::Rotate90Degrees()
{
	IntVec3 rotated = GetRotated90Degrees();
	x = rotated.x;
	y = rotated.y;
	z = rotated.z;
}

void IntVec3::RotateMinus90Degrees()
{
	IntVec3 negativeRotated = GetRotatedMinus90Degrees();
	x = negativeRotated.x;
	y = negativeRotated.y;
	z = negativeRotated.z;
}

void IntVec3::SetFromText(char const* text)
{
	Strings components = SplitStringOnDelimiter(text, ',');

	if (components.size() == 2)
	{
		x = std::stoi(components[0].c_str());
		y = std::stoi(components[1].c_str());
		z = std::stoi(components[2].c_str());
	}
	else
	{
		x = y = z = 0;
	}
}

void IntVec3::operator=(const IntVec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

bool IntVec3::operator==(const IntVec3& compare) const
{
	return compare.x == x && compare.y == y && compare.z == z;
}

const IntVec3 IntVec3::operator+(const IntVec3& intVec3ToAdd) const
{
	return IntVec3( intVec3ToAdd.x + x, intVec3ToAdd.y + y, intVec3ToAdd.z + z);
}

void IntVec3::operator-=(const IntVec3& intVecToSubtract)
{
	x -= intVecToSubtract.x;
	y -= intVecToSubtract.y;
	z -= intVecToSubtract.z;
}

const IntVec3 IntVec3::operator*(int scalar) const
{
	return IntVec3(x * scalar, y * scalar, z * scalar);
}

void IntVec3::operator+=(const IntVec3& intVecToAdd)
{
	x += intVecToAdd.x;
	y += intVecToAdd.y;
	z += intVecToAdd.z;
}

const IntVec3 IntVec3::operator-(const IntVec3& intVecToSubtract) const
{
	return IntVec3(x - intVecToSubtract.x, y - intVecToSubtract.y, z - intVecToSubtract.z);
}

const IntVec3 IntVec3::operator-() const
{
	return IntVec3( -x, -y, -z );
}
