#pragma once

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

	static IntVec3 const ZERO;
	static IntVec3 const NORTH;
	static IntVec3 const SOUTH;
	static IntVec3 const EAST;
	static IntVec3 const WEST;
	static IntVec3 const SKY;
	static IntVec3 const GROUND;

public:
	IntVec3();
	~IntVec3();
	IntVec3(const IntVec3& copyFrom);
	explicit IntVec3(int initialX, int initialY, int initialZ);

	// Const Methods 
	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;

	IntVec3 const GetRotated90Degrees() const;
	IntVec3 const GetRotatedMinus90Degrees() const;

	// Non-const Methods 
	void Rotate90Degrees();
	void RotateMinus90Degrees();
	void SetFromText(char const* text);

	// Operators (self-mutating / non-const)
	void operator=(const IntVec3& copyFrom);  // IntVec3 = IntVec3
	bool operator==(const IntVec3& compare) const;
	const IntVec3	operator+(const IntVec3& intVec3ToAdd) const; // IntVec3 + IntVec3
	void operator-=(const IntVec3& intVecToSubtract);		// IntVec3 -= IntVec3
	void operator+=(const IntVec3& intVecToAdd);		// IntVec3 += IntVec3
	const IntVec3	operator-(const IntVec3& intVecToSubtract) const;	// IntVec3 - IntVec3
	const IntVec3	operator-() const;
	const IntVec3 operator*(int scalar) const;
};