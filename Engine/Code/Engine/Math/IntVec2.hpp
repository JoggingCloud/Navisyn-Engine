#pragma once
#include <functional>
#include <utility>

struct IntVec2
{
public:
	int x = 0;
	int y = 0;

	static const IntVec2 INVALID_POSITION;
	
	static IntVec2 const ZERO;
	static IntVec2 const NORTH;
	static IntVec2 const SOUTH;
	static IntVec2 const EAST;
	static IntVec2 const WEST;

public:
	IntVec2();
	~IntVec2();
	IntVec2(const IntVec2& copyFrom);
	explicit IntVec2(int initialX, int initialY);

	// Const Methods 
	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;

	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;

	// Non-const Methods 
	void Rotate90Degrees();
	void RotateMinus90Degrees();
	void SetFromText(char const* text);

	// Operators (self-mutating / non-const)
	void operator=(const IntVec2& copyFrom);  // IntVec2 = IntVec2
	bool operator==(const IntVec2& compare) const;
	bool operator!=(const IntVec2& compare) const;
	const IntVec2	operator+( const IntVec2& intVec2ToAdd ) const; // IntVec2 + IntVec2
	void operator-=( const IntVec2& intVecToSubtract );		// IntVec2 -= IntVec2
	void operator+=( const IntVec2& intVecToAdd );		// IntVec2 += IntVec2
	const IntVec2	operator-(const IntVec2& intVecToSubtract) const;	// IntVec2 - IntVec2
	const IntVec2	operator-() const;								// -IntVec2, i.e. "unary negation"
	friend bool operator<(IntVec2 const& a, IntVec2 const& b);
};

inline int GetLengthSquared(const IntVec2& pos1, const IntVec2& pos2)
{
	int distanceX = pos2.x - pos1.x;
	int distanceY = pos2.y - pos1.y;
	return (distanceX * distanceX) + (distanceY * distanceY);
}

struct IntVec2Hash
{
	std::size_t operator()(const IntVec2& v) const noexcept; // Perform compile-time check for exceptions
};

struct IntVec2PairHash
{
	std::size_t operator()(const std::pair<IntVec2, IntVec2>& pair) const noexcept;
};

struct IntVec2PairEqual
{
	bool operator()(const std::pair<IntVec2, IntVec2>& leftHash, const std::pair<IntVec2, IntVec2>& rightHash) const noexcept;
};

namespace std
{
	template <>
	struct hash<IntVec2>
	{
		size_t operator()(const IntVec2& vec) const
		{
			return (hash<int>()(vec.x) ^ (hash<int>()(vec.y) << 1));
		}
	};
}