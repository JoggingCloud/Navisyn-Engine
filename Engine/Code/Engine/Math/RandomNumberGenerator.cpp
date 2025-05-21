#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"
#include <ctime>
#include <random>
#include <cstdlib>

int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	return (randomUInt % maxNotInclusive);
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	int range = 1 + (maxInclusive - minInclusive);
	return minInclusive + (rand() % range);
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	int randomInt = rand();
	float randFloat = static_cast<float>(randomInt) / static_cast<float>(RAND_MAX);
	return randFloat;
}

float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	int randomInt = rand();
	float randFloat = static_cast<float>(randomInt) / static_cast<float>(RAND_MAX);
	float range = (maxInclusive - minInclusive);
	return (randFloat * range) + minInclusive;
}

Vec3 RandomNumberGenerator::RollRandomVec3InRange(float minInclusive, float maxInclusive)
{
	float x = RollRandomFloatInRange(minInclusive, maxInclusive);
	float y = RollRandomFloatInRange(minInclusive, maxInclusive);
	float z = RollRandomFloatInRange(minInclusive, maxInclusive);

	return Vec3(x, y, z);
}

Vec2 RandomNumberGenerator::RollRandomVec2InRange(float minInclusive, float maxInclusive)
{
	float x = RollRandomFloatInRange(minInclusive, maxInclusive);
	float y = RollRandomFloatInRange(minInclusive, maxInclusive);

	return Vec2(x, y);
}

IntVec2 RandomNumberGenerator::RollRandomIntVec2InRange(int minInclusive, int maxInclusive)
{
	int x = RollRandomIntInRange(minInclusive, maxInclusive);
	int y = RollRandomIntInRange(minInclusive, maxInclusive);

	return IntVec2(x, y);
}

// --------------------------------------------------------------------------------------------------------------------------------------

int RandomNumberGenerator::SRollRandomIntInRange(int minInclusive, int maxInclusive)
{
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	int range = 1 + (maxInclusive - minInclusive);
	return minInclusive + (randomUInt % range);
}

unsigned int RandomNumberGenerator::SRollRandomUnsignedInt()
{
	return Get1dNoiseUint(m_position++, m_seed);
}

float RandomNumberGenerator::SRollRandomFloatZeroToOne()
{
	return Get1dNoiseZeroToOne(m_position++, m_seed);
}

float RandomNumberGenerator::SRollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	unsigned int randomUint = Get1dNoiseUint(m_position++, m_seed);
	float randFloat = static_cast<float>(randomUint) / static_cast<float>(UINT_MAX);
	float range = (maxInclusive - minInclusive);
	return (randFloat * range) + minInclusive;
}

IntVec2 RandomNumberGenerator::SRollRandomIntVec2InRange(int minInclusive, int maxInclusive)
{
	int x = SRollRandomIntInRange(minInclusive, maxInclusive);
	int y = SRollRandomIntInRange(minInclusive, maxInclusive);

	return IntVec2(x, y);
}