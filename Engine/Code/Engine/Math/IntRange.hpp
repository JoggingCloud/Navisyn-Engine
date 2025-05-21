#pragma once

class IntRange
{
public:
	int m_min;
	int m_max;

public:
	~IntRange();
	IntRange(); // Default Constructor to the range [0, 0]
	explicit IntRange(int min, int max); // Explicit Constructor to a range [min, max]

											   // Operators (const)
	bool		operator==(const IntRange& compare) const;		// int == int
	bool		operator!=(const IntRange& compare) const;		// int != int

																	// Operators (self-mutating / non-const)
	void		operator=(const IntRange& copyFrom);				// int = int

public:
	bool IsOnRange(int value) const;
	bool IsOverlapingWith(const IntRange& compare) const;

public:
	// Static Const objects 
	static const IntRange ZERO;
	static const IntRange ONE;
	static const IntRange ZERO_TO_ONE;
};