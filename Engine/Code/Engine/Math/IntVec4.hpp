#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <math.h>
#include <functional>
#include <utility>

template <typename T>
struct IntVec4
{
	T x = 0;
	T y = 0;
	T z = 0;
	T w = 0;

	IntVec4() = default;
	~IntVec4() = default;
	explicit IntVec4(T initialX, T initialY, T initialZ, T initialW)
		: x(initialX), y(initialY), z(initialZ), w(initialW) {}

	// Const Methods 
	float GetLength() const;
	T GetLengthSquared() const;

	// Non-const Methods 
	void SetFromText(char const* text);

	// Operators (self-mutating / non-const)
	void operator=(const IntVec4<T>& copyFrom);  // IntVec4 = IntVec4
	bool operator==(const IntVec4<T>& compare) const;
	const IntVec4<T> operator+(const IntVec4<T>& intVec4ToAdd) const; // IntVec4 + IntVec4
	void operator-=(const IntVec4<T>& intVecToSubtract);		// IntVec4 -= IntVec4
	void operator+=(const IntVec4<T>& intVecToAdd);		// IntVec4 += IntVec4
	const IntVec4<T> operator-(const IntVec4<T>& intVecToSubtract) const;	// IntVec4 - IntVec4
	const IntVec4<T> operator-() const;
	const IntVec4<T> operator*(T scalar) const;
};

template <typename T>
float IntVec4<T>::GetLength() const
{
	return std::sqrt(static_cast<float>(x * x) + static_cast<float>(y * y) + static_cast<float>(z * z) + static_cast<float>(w * w));
}

template <typename T>
T IntVec4<T>::GetLengthSquared() const
{
	return (x * x) + (y * y) + (z * z) + (w * w);
}

template <typename T>
void IntVec4<T>::operator=(const IntVec4<T>& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

template <typename T>
bool IntVec4<T>::operator==(const IntVec4<T>& compare) const
{
	return compare.x == x && compare.y == y && compare.z == z && compare.w == w;
}

template <typename T>
const IntVec4<T> IntVec4<T>::operator+(const IntVec4<T>& intVec4ToAdd) const
{
	return IntVec4<T>(x + intVec4ToAdd.x, y + intVec4ToAdd.y, z + intVec4ToAdd.z, w + intVec4ToAdd.w);
}

template <typename T>
const IntVec4<T> IntVec4<T>::operator-(const IntVec4<T>& intVecToSubtract) const
{
	return IntVec4<T>(x - intVecToSubtract.x, y - intVecToSubtract.y, z - intVecToSubtract.z, w - intVecToSubtract.w);
}

template <typename T>
const IntVec4<T> IntVec4<T>::operator-() const
{
	return IntVec4<T>(-x, -y, -z, -w);
}

template <typename T>
const IntVec4<T> IntVec4<T>::operator*(T scalar) const
{
	return IntVec4<T>(x * scalar, y * scalar, z * scalar, w * scalar);
}

template <typename T>
void IntVec4<T>::operator+=(const IntVec4<T>& intVecToAdd)
{
	x += intVecToAdd.x;
	y += intVecToAdd.y;
	z += intVecToAdd.z;
	w += intVecToAdd.w;
}

template <typename T>
void IntVec4<T>::operator-=(const IntVec4<T>& intVecToSubtract)
{
	x -= intVecToSubtract.x;
	y -= intVecToSubtract.y;
	z -= intVecToSubtract.z;
	w -= intVecToSubtract.w;
}

template <typename T>
void IntVec4<T>::SetFromText(char const* text)
{
	Strings components = SplitStringOnDelimiter(text, ',');

	if (components.size() == 4)
	{
		x = static_cast<T>(std::stoi(components[0].c_str()));
		y = static_cast<T>(std::stoi(components[1].c_str()));
		z = static_cast<T>(std::stoi(components[2].c_str()));
		w = static_cast<T>(std::stoi(components[3].c_str()));
	}
	else
	{
		x = y = z = w = 0;
	}
}

namespace std 
{
	template<typename T>
	struct hash<IntVec4<T>>
	{
		std::size_t operator()(const IntVec4<T>& v) const noexcept
		{
			std::size_t hx = std::hash<T>{}(v.x);
			std::size_t hy = std::hash<T>{}(v.y);
			std::size_t hz = std::hash<T>{}(v.z);
			std::size_t hw = std::hash<T>{}(v.w);
			return hx ^ (hy << 1) ^ (hz << 2) ^ (hw << 3); // Combine hashes
		}
	};
}
