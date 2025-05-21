#pragma once
#include "Engine/Math/Plane.hpp"

struct ConvexHull2D
{
	std::vector<Plane2D> m_boundingplanes;
};