#include "Engine/Math/Vec4.hpp"
#include <math.h>

Vec4 const Vec4::ZERO = Vec4(0.f, 0.f, 0.f, 0.f);

Vec4::Vec4(float initialX, float initialY, float initialZ, float initialW)
	: x(initialX),
	  y(initialY),
	  z(initialZ),
	  w(initialW)
{
}

Vec4::Vec4(Vec3 copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

Vec4 Vec4::Rgba8ToVec4(const Rgba8& color)
{
	return Vec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
}

const Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}

bool Vec4::operator<(const Vec4& compare) const
{
	if (x != compare.x)
	{
		return x < compare.x;
	}
	if (y != compare.y)
	{
		return y < compare.y;
	}
	if (z != compare.z)
	{
		return z < compare.z;
	}
	return w < compare.w;
}

bool Vec4::operator==(const Vec4& compare) const
{
	return compare.x == x && compare.y == y && compare.z == z && compare.w == w;
}

bool Vec4::operator!=(const Vec4& compare) const
{
	return compare.x != x || compare.y != y || compare.z != z || compare.w != w;
}

const Vec4 Vec4::operator+(const Vec4& vecToAdd) const
{
	return Vec4(vecToAdd.x + x, vecToAdd.y + y, vecToAdd.z + z, vecToAdd.w + w);
}

const Vec4 Vec4::operator-(const Vec4& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}

const Vec4 Vec4::operator*(float uniformScale) const
{
	return Vec4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale);
}

const Vec4 Vec4::operator/(float inverseScale) const
{
	return Vec4(x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale);
}

void Vec4::operator+=(const Vec4& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}

void Vec4::operator-=(const Vec4& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}

void Vec4::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}

void Vec4::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
	w /= uniformDivisor;
}

void Vec4::operator=(const Vec4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

const Vec4 operator*(float uniformScale, const Vec4& vecToScale)
{
	return Vec4(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z, uniformScale * vecToScale.w);
}
