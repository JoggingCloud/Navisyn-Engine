#pragma once
#include "Engine/Math/Vec2.hpp"
#include <unordered_map>

 struct Vec3
 {
 public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
 	float x = 0.f;
 	float y = 0.f;
 	float z = 0.f;

	static const Vec3 INVALID_POSITION;

	static const Vec3 ZERO;
	static const Vec3 NORTH;
	static const Vec3 SOUTH;
	static const Vec3 EAST;
	static const Vec3 WEST;
	static const Vec3 SKY;
	static const Vec3 GROUND;

 public:
	 explicit Vec3(Vec2 copyFrom);
	 Vec3() = default;
	 ~Vec3() {}
     explicit Vec3(float initialX, float initialY, float initialZ);

	 // Non const
	 void Normalize();
	 Vec3 const GetNormalized() const;

	 void SetFromText(char const* text);

	 float GetChebyshevDistance3D(Vec3 const& start, Vec3 const& end);

	 bool IsValid() const;
	 bool IsNearlyZero() const;
	
     // Accessors (const methods)
     float GetLength() const;
     float GetLengthXY() const;
     float GetLengthSquared() const;
     float GetLengthXYSquared() const;
     float GetAngleAboutZRadians() const;
     float GetAngleAboutZDegrees() const;
     Vec3 const GetRotatedAboutZRadians(float deltaRadians) const;
     Vec3 const GetRotatedAboutZDegrees(float deltaDegrees) const;
	 Vec3 const GetRotatedAboutXDegrees(float deltaDegrees) const;
     Vec3 const GetClamped(float maxLength) const;
	 Vec3 GetAbsolute() const;

	 // Static methods (e.g. creation functions)
	 static Vec3 Min(Vec3 const& a, Vec3 const& b);
	 static Vec3 Max(Vec3 const& a, Vec3 const& b);
	 static Vec3 const MakeFromPolarRadians(float latitudeRadians, float longitudeRadians, float length = 1.f);
	 static Vec3 const MakeFromPolarDegrees(float latitudeDegrees, float longitudeDegrees, float length = 1.f);

	 const Vec3	operator-() const;	

	 // Operators (const)
	 bool		operator<(const Vec3& compare) const;				// vec3 < vec3
	 bool		operator>(const Vec3& compare) const;				// vec3 < vec3
	 bool		operator==(const Vec3& compare) const;			// vec3 == vec3
	 bool		operator!=(const Vec3& compare) const;			// vec3 != vec3
	 Vec3 const	operator+(const Vec3& vecToAdd) const;			// vec3 + vec3
	 Vec3 const	operator-(const Vec3& vecToSubtract) const;		// vec3 - vec3
	 Vec3 const	operator*(float uniformScale) const;			// vec3 * float
	 Vec3 const	operator/(float inverseScale) const;			// vec3 / float

	// Operators (self-mutating / non-const)
	 void		operator+=(const Vec3& vecToAdd);			// vec3 += vec3
	 void		operator-=(const Vec3& vecToSubtract);		// vec3 -= vec3
	 void		operator*=(const float uniformScale);		// vec3 *= float
	 void		operator/=(const float uniformDivisor);		// vec3 /= float
	 void		operator=(const Vec3& copyFrom);			// vec3 = vec3

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend const Vec3 operator*(float uniformScale, const Vec3& vecToScale);	// float * vec3
 };

 inline float GetLengthSquared(const Vec3& pos1, const Vec3& pos2)
 {
	 float distanceX = pos2.x - pos1.x;
	 float distanceY = pos2.y - pos1.y;
	 float distanceZ = pos2.z - pos1.z;
	 return (distanceX * distanceX) + (distanceY * distanceY) + (distanceZ * distanceZ);
 }

 struct Vec3Hash
 {
	 std::size_t operator()(const Vec3& v) const noexcept; // Perform compile-time check for exceptions
 };

 struct Vec3PairHash
 {
	 std::size_t operator()(const std::pair<Vec3, Vec3>& pair) const noexcept;
 };

 struct Vec3PairEqual
 {
	 bool operator()(const std::pair<Vec3, Vec3>& leftHash, const std::pair<Vec3, Vec3>& rightHash) const noexcept;
 };

 namespace std 
 {
	 template <>
	 struct hash<Vec3> 
	 {
		 std::size_t operator()(const Vec3& vec) const noexcept 
		 {
			 std::size_t h1 = std::hash<float>{}(vec.x);
			 std::size_t h2 = std::hash<float>{}(vec.y);
			 std::size_t h3 = std::hash<float>{}(vec.z);
			 return h1 ^ (h2 << 1) ^ (h3 << 2);
		 }
	 };
 }