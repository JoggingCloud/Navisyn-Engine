#pragma once
#include <mutex>
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/LineSegment3.hpp"

enum class DebugRenderMode
{
	ALWAYS,
	USE_DEPTH,
	X_RAY,
};

struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr;
	std::string m_fontName = "SquirrelFixedFont";
};

// Setup
void DebugRenderSystemStartup(const DebugRenderConfig& config);
void DebugRenderSystemShutdown();

// Control
void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();

// Output
void DebugRenderBeginFrame();
void DebugRenderWorld(const Camera& camera);
void DebugRenderScreen(const Camera& camera);
void DebugRenderEndFrame();

// Geometry
void DebugAddWorld3DTriangle(const Vec3& position1, const Vec3& position2, const Vec3& position3, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld3DWireTriangle(const Vec3& position1, const Vec3& position2, const Vec3& position3, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld3DTriangle(const Mat44& transform, const Vec3& direction, float zPosition, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld3DWireTriangle(const Mat44& transform, const Vec3& direction, float zPosition, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld2DRing(const Vec2& center, float radius, int sides, float thickness, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld3DRing(const Vec3& center, float radius, int sides, float thickness, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldQuad(float duration, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldLineSegmentedQuad(float duration, float lineThickness, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldWireQuad(float duration, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldWireAABB3D(float duration, const AABB3& bounds, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldPoint(const Vec3& pos, float radius, int numSlices, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldLine(const Vec3& start, const Vec3& end, float lineThickness, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldLine(const Mat44& transform, float lineLength, float radius, int slices, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld2DDisc(const Vec2& center, float radius, int sides, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld3DDisc(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld2DCone(float duration, const Mat44& transform, float radius, float coneAngleDegrees, float startAngleDegrees, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorld3DCone(float duration, const Mat44& transform, float radius, float conelength, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float interpolate, float coneRadius, float cylinderRadius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldWireArrow(const Vec3& start, const Vec3& end, float interpolate, float coneRadius, float cylinderRadius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAdd2DWorldBasis(const Mat44& transform, float interpolate, float cylinderRadius, float coneRadius, float basisLength, float duration, DebugRenderMode mode);
void DebugAdd3DWorldBasis(const Mat44& transform, float interpolate, float cylinderRadius, float coneRadius, float basisLength, float duration, DebugRenderMode mode);
void DebugAddScreenText(const std::string& text, const Vec2& position, float size, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor);
void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor);

// Console commands
bool Command_DebugRenderClear(EventArgs& args);
bool Command_DebugRenderToggle(EventArgs& args);