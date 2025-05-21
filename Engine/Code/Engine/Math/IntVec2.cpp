#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <math.h>

IntVec2 const IntVec2::INVALID_POSITION = IntVec2(0, 0);
IntVec2 const IntVec2::ZERO  = IntVec2(0, 0);
IntVec2 const IntVec2::NORTH = IntVec2(0, 1);
IntVec2 const IntVec2::SOUTH = IntVec2(0, -1);
IntVec2 const IntVec2::EAST  = IntVec2(1, 0);
IntVec2 const IntVec2::WEST  = IntVec2(-1, 0);

IntVec2::IntVec2()
{

}

IntVec2::~IntVec2()
{

}

IntVec2::IntVec2(const IntVec2& copyFrom)
	:x(copyFrom.x)
	,y(copyFrom.y)
{
}

IntVec2::IntVec2(int initialX, int initialY)
	:x(initialX)
	,y(initialY)
{
}

// Const Methods 
float IntVec2::GetLength() const
{
	return sqrtf(static_cast<float>(x * x) + static_cast<float>(y * y));
}

int IntVec2::GetTaxicabLength() const
{
	return abs(x) + abs(y);
}

int IntVec2::GetLengthSquared() const
{
	return ((x * x) + (y * y));
}

float IntVec2::GetOrientationRadians() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x));
}

float IntVec2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees(atan2f(static_cast<float>(y), static_cast<float>(x)));
}

const IntVec2 IntVec2::GetRotated90Degrees() const
{
	return IntVec2(-y, x);
}

const IntVec2 IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2(y,-x);
}

// Non-const Methods
void IntVec2::Rotate90Degrees()
{
	IntVec2 rotated = GetRotated90Degrees();
	x = rotated.x;
	y = rotated.y;
}

void IntVec2::RotateMinus90Degrees()
{
	IntVec2 negativeRotated = GetRotatedMinus90Degrees();
	x = negativeRotated.x;
	y = negativeRotated.y;
}

void IntVec2::SetFromText(char const* text)
{
	Strings components = SplitStringOnDelimiter(text, ',');

	if (components.size() == 2)
	{
		x = std::stoi(components[0].c_str());
		y = std::stoi(components[1].c_str());
	}
	else
	{
		x = y = 0;
	}
}

const IntVec2 IntVec2::operator+(const IntVec2& intVec2ToAdd) const
{
	return IntVec2( intVec2ToAdd.x + x, intVec2ToAdd.y + y);
}

void IntVec2::operator+=(const IntVec2& intVecToAdd)
{
	x += intVecToAdd.x;
	y += intVecToAdd.y;
}

void IntVec2::operator-=(const IntVec2& intVecToSubtract)
{
	x -= intVecToSubtract.x;
	y -= intVecToSubtract.y;
}

const IntVec2 IntVec2::operator-(const IntVec2& intVecToSubtract) const
{
	return IntVec2(x - intVecToSubtract.x, y - intVecToSubtract.y);
}

const IntVec2 IntVec2::operator-() const
{
	return IntVec2( -x, -y );
}

bool IntVec2::operator==(const IntVec2& compare) const
{
	return compare.x == x && compare.y == y;
}

bool IntVec2::operator!=(const IntVec2& compare) const
{
	return compare.x != x || compare.y != y;
}

// Operators (self-mutating / non-const)
void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

bool operator<(IntVec2 const& a, IntVec2 const& b)
{
	if (a.y < b.y)
	{
		return true;
	}
	else if (b.y < a.y)
	{
		return false;
	}
	else
	{
		return a.x < b.x;
	}
}

std::size_t IntVec2Hash::operator()(const IntVec2& v) const noexcept // Hash function for IntVec2
{
	return std::hash<int>()(v.x) ^ (std::hash<int>{}(v.x) << 1); // Combine the hash of x and y to generate a hash for IntVec2
}

std::size_t IntVec2PairHash::operator()(const std::pair<IntVec2, IntVec2>& pair) const noexcept
{
	return IntVec2Hash()(pair.first) ^ (IntVec2Hash()(pair.second) << 1);
}

bool IntVec2PairEqual::operator()(const std::pair<IntVec2, IntVec2>& leftHash, const std::pair<IntVec2, IntVec2>& rightHash) const noexcept
{
	return leftHash.first == rightHash.first && leftHash.second == rightHash.second;
}
