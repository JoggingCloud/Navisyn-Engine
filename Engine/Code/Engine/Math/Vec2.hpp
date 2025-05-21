#pragma once
#include <functional>

//-----------------------------------------------------------------------------------------------
struct Vec2
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;

	static const Vec2 ZERO;

public:
	// Construction/Destruction
	~Vec2() {}												// destructor (do nothing)
	Vec2() {}												// default constructor (do nothing)
	Vec2( const Vec2& copyFrom );							// copy constructor (from another vec2)
	explicit Vec2( float initialX, float initialY );		// explicit constructor (from x, y)

	static Vec2 const MakeFromPolarRadians(float orientationRadians, float length = 1.f);

	// Operators (const)
	bool		operator==( const Vec2& compare ) const;		// vec2 == vec2
	bool		operator!=( const Vec2& compare ) const;		// vec2 != vec2
	bool		operator<( const Vec2& compare) const;
	const Vec2	operator+( const Vec2& vecToAdd ) const;		// vec2 + vec2
	const Vec2	operator-( const Vec2& vecToSubtract ) const;	// vec2 - vec2
	const Vec2	operator-() const;								// -vec2, i.e. "unary negation"
	const Vec2	operator*( float uniformScale ) const;			// vec2 * float
	const Vec2	operator*( const Vec2& vecToMultiply ) const;	// vec2 * vec2
	const Vec2	operator/( float inverseScale ) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=( const Vec2& vecToAdd );				// vec2 += vec2
	void		operator-=( const Vec2& vecToSubtract );		// vec2 -= vec2
	void		operator*=( const float uniformScale );			// vec2 *= float
	void		operator/=( const float uniformDivisor );		// vec2 /= float
	void		operator=( const Vec2& copyFrom );				// vec2 = vec2

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend const Vec2 operator*( float uniformScale, const Vec2& vecToScale );	// float * vec2

	// Static methods (e.g. creation functions)
	static Vec2 const MakeFromPolarDegrees(float orientationDegrees, float length = 1.f);

	// Non Const Methods 
	void SetOrientationDegrees(float orientationDegrees);
	void SetOrientationRadians(float orientationRadians);

	void SetPolarDegrees(float newOrientationDegrees, float newLength);
	void SetPolarRadians(float newOrientationRadians, float newLength);
	
	void Rotate90Degrees();
	void RotateMinus90Degrees();

	void RotateRadians(float deltaRadians);
	void RotateDegrees(float deltaDegrees);
	
	void SetLength(float newLength);
	void ClampLength(float maxLength);
	
	void Normalize();
	float NormalizeAndGetPreviousLength();

	void Reflect(Vec2 const& impactSurfaceNormal);

	void SetFromText(char const* text);

	static Vec2 GetRandomDirection2D();

	// Const Methods
	Vec2 Lerp(const Vec2& other, float t) const;
	Vec2 const GetReflected(Vec2 const& impactSurfaceNormal) const;
	
	float GetLength() const;
	float GetLengthSquared() const;

	float GetOrientationDegrees() const;
	float GetOrientationRadians() const;

	Vec2 const GetRotated90Degrees() const;
	Vec2 const GetRotatedMinus90Degrees() const;

	Vec2 const GetRotatedRadians(float deltaRadians) const;
	Vec2 const GetRotatedDegrees(float deltaDegrees) const;

	Vec2 const GetClamped(float maxLength) const;
	Vec2 const GetNormalized() const;
};

namespace std 
{
	template <> struct hash<Vec2> 
	{
		std::size_t operator()(Vec2 const& vec) const
		{
			std::size_t h1 = std::hash<float>()(vec.x); 
			std::size_t h2 = std::hash<float>()(vec.y);
			return h1 ^ (h2 << 1);
		}
	};
}