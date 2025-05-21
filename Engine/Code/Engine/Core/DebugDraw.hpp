#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

class Renderer;

extern Renderer* g_theRenderer;

void DebugDrawLine(Vec2 startPoint, Vec2 endPoint, float thickness, Rgba8 color);
void DebugDraw2DRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);