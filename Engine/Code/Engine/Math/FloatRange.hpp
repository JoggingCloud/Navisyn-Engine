#pragma once


class FloatRange
{
public:
	float m_min;
	float m_max;

public:
	~FloatRange();
	FloatRange(); // Default Constructor to the range [0, 0]
	explicit FloatRange(float min, float max); // Explicit Constructor to a range [min, max]

	// Operators (const)
	bool		operator==(const FloatRange& compare) const;		// float == float
	bool		operator!=(const FloatRange& compare) const;		// float != float

	// Operators (self-mutating / non-const)
	void		operator=(const FloatRange& copyFrom);				// float = float

public:
	float GetRandomValueInRange() const;
	bool IsOnRange(float value) const;
	bool IsOverlapingWith(const FloatRange& compare) const;
	void SetFromText(char const* text);

public:
	// Static Const objects 
	static const FloatRange ZERO;
	static const FloatRange ONE;
	static const FloatRange ZERO_TO_ONE;
};