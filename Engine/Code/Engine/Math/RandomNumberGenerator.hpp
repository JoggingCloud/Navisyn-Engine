#pragma once
#include <vector>

class Clock;
struct Vec3;
struct Vec2;
struct IntVec2;

class RandomNumberGenerator
{
public:
	RandomNumberGenerator(unsigned int seed = 0) : m_seed(seed) {};
	
	int RollRandomIntLessThan(int maxNotInclusive);
	int RollRandomIntInRange(int minInclusive, int maxInclusive);
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive);
	Vec3 RollRandomVec3InRange(float minInclusive, float maxInclusive);
	Vec2 RollRandomVec2InRange(float minInclusive, float maxInclusive);
	IntVec2 RollRandomIntVec2InRange(int minInclusive, int maxInclusive);
	
	
	int SRollRandomIntInRange(int minInclusive, int maxInclusive);
	unsigned int SRollRandomUnsignedInt();
	float SRollRandomFloatZeroToOne();
	float SRollRandomFloatInRange(float minInclusive, float maxInclusive);
	IntVec2 SRollRandomIntVec2InRange(int minInclusive, int maxInclusive);
	
	void SetSeed(unsigned int newSeed) { m_seed = newSeed; m_position = 0; }

	template<typename T>
	T const& GetRandomElement(std::vector<T> const& elements);

public:
	unsigned int m_seed = 0;
	int m_position = 0;
};

template<typename T>
T const& RandomNumberGenerator::GetRandomElement(std::vector<T> const& elements)
{
	int index = SRollRandomIntInRange(0, static_cast<int>(elements.size()) - 1);
	return elements[index];
}
