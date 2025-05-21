#pragma once
#include"Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

struct Vec4 
{
public: 
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	static const Vec4 ZERO;

public:
	explicit Vec4(Vec3 copyFrom);
	Vec4() {}
	~Vec4() {}
	explicit Vec4(float initialX, float initialY, float initialZ, float initialW);
	static Vec4 Rgba8ToVec4(const Rgba8& color);
	const Vec4	operator-() const;

	// Operators (const)
	bool		operator<(const Vec4& compare) const;			// vec4 < vec4
	bool		operator==(const Vec4& compare) const;			// vec4 == vec4
	bool		operator!=(const Vec4& compare) const;			// vec4 != vec4
	Vec4 const	operator+(const Vec4& vecToAdd) const;			// vec4 + vec4
	Vec4 const	operator-(const Vec4& vecToSubtract) const;		// vec4 - vec4
	Vec4 const	operator*(float uniformScale) const;			// vec4 * float
	Vec4 const	operator/(float inverseScale) const;			// vec4 / float

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec4& vecToAdd);			// vec4 += vec4
	void		operator-=(const Vec4& vecToSubtract);		// vec4 -= vec4
	void		operator*=(const float uniformScale);		// vec4 *= float
	void		operator/=(const float uniformDivisor);		// vec4 /= float
	void		operator=(const Vec4& copyFrom);			// vec4 = vec4

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec4::
	friend const Vec4 operator*(float uniformScale, const Vec4& vecToScale);	// float * vec4
};