#pragma once
#include "Engine/Math/MathUtils.hpp"
#include <functional>

class GridPathfindingManager;
extern GridPathfindingManager* g_gridPathfindingManager;

constexpr int MAX_DIST_THRESHOLD = 32;

enum class DirectionMode
{
	Cardinal4,
	Cardinal8
};

enum CardinalDir
{
	EAST,
	NORTH,
	WEST,
	SOUTH,
	NUM_CARDINAL_DIRECTIONS
};

enum IntercardinalDir
{
	IEAST,
	NORTHEAST,
	INORTH,
	NORTHWEST,
	IWEST,
	SOUTHWEST,
	ISOUTH,
	SOUTHEAST,
	NUM_INTERCARDINAL_DIRECTIONS
};

inline IntVec2 GetStepInCardinalDirection(CardinalDir direction)
{
	switch (direction)
	{
	case EAST:  return IntVec2(1, 0);
	case NORTH: return IntVec2(0, 1);
	case WEST:  return IntVec2(-1, 0);
	case SOUTH: return IntVec2(0, -1);
	case NUM_CARDINAL_DIRECTIONS: return IntVec2(0, 0);
	default: return IntVec2(0, 0);
	}
};

inline IntVec2 GetStepInCardinalAndIntercardinalDirection(IntercardinalDir direction)
{
	switch (direction)
	{
	case IEAST:      return IntVec2(1, 0);
	case NORTHEAST:  return IntVec2(1, 1);
	case INORTH:     return IntVec2(0, 1);
	case NORTHWEST:  return IntVec2(-1, 1);
	case IWEST:      return IntVec2(-1, 0);
	case SOUTHWEST:  return IntVec2(-1, -1);
	case ISOUTH:     return IntVec2(0, -1);
	case SOUTHEAST:  return IntVec2(1, -1);
	case NUM_INTERCARDINAL_DIRECTIONS: return IntVec2(0, 0);
	default: return IntVec2(0, 0);
	}
};
