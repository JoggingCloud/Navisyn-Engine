#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/LineSegment3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/Mat44.hpp"
#include <vector>

void CalculateTangentSpaceBasisVectors(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, bool computeNormals = true, bool computeTangents = true);
void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform);
void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform, bool hasNormals = false);
AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts);
AABB3 GetVertexBounds3D(const std::vector<Vertex_PCU>& verts);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
void AddVertsFor3DTriangle(std::vector<Vertex_PCU>& verts, Vec3 const& startPos, float radius, Rgba8 const& color);
void AddVertsFor3DTriangle(std::vector<Vertex_PCU>& verts, Vec3 const& startPos, Vec3 const& direction, float radius, Rgba8 const& color);
void AddVertsFor3DTriangle(std::vector<Vertex_PCU>& verts, Vec3 const& position1, Vec3 const& position2, Vec3 const& position3, Rgba8 const& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsFor3DTriangle(std::vector<Vertex_PCUTBN>& verts,  std::vector<unsigned int>& indexes, Vec3 const& position1, Vec3 const& position2, Vec3 const& position3, Rgba8 const& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsFor3DLinedTriangle(std::vector<Vertex_PCU>& verts, const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3, float lineThickness, Rgba8 const& color);
void AddVertsFor3DLinedTriangle(std::vector<Vertex_PCU>& verts, const LineSegment3& lineSegment1, const LineSegment3& lineSegment2, const LineSegment3& lineSegment3, float lineThickness, Rgba8 const& color);
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, int sides, Rgba8 const& color);
void AddVertsForDisc3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, int sides, Rgba8 const& color);
void AddVertsFor3DRing(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, int sides, float thickness, Rgba8 const& color);
void AddVertsForHexagon3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float width, float thickness, Rgba8 const& color);
void AddVertsForHollowHexagon3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float width, float thickness, Rgba8 const& color);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, const AABB2& bounds, float lineThickness, const Rgba8& color = Rgba8::WHITE);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color=Rgba8::WHITE, Vec2 const& uvAtMins = Vec2(0,0), Vec2 const& uvAtMaxs = Vec2(1,1));
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddvertsForLineSegment2D(std::vector<Vertex_PCU>& verts, const LineSegment2& lineSegment, float thickness, Rgba8 const& color);
void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float lineThickness, Rgba8 const& color);
void AddvertsForLineSegmentAABB3D(std::vector<Vertex_PCU>& verts, const LineSegment3& lineSegment, float thickness, Rgba8 const& color);
void AddVertsForLineSegmentOBB3D(std::vector<Vertex_PCU>& verts, const LineSegment3& lineSegment, float thickness, Rgba8 const& color);
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color);
void AddVertsForCone2D(std::vector<Vertex_PCU>& verts, const Vec2& center, float radius, float angleDegrees, float startAngleDegrees, const Rgba8& color);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, float thickness, const Rgba8& color);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, float lineThickness, const Rgba8& color = Rgba8::WHITE);
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, const OBB3& box, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const OBB3& box, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForZSphere(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForZSphere(std::vector<Vertex_PCUTBN>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForZSphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForZSphere(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForZSphere(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices);
void AddVertsForSphere(std::vector<Vertex_PCU>& verts, const Mat44& transform, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForSphere(std::vector<Vertex_PCU>& verts, const Mat44& transform, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForNorthZHemisphere(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForNorthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForNorthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices);
void AddVertsForSouthZHemisphere(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForSouthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices);
void AddVertsForSouthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices);
void AddVertsForHemisphere(std::vector<Vertex_PCU>& verts, const Mat44& transform, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, bool isNorthHemisphere);
void AddVertsForHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Mat44& transform, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, bool isNorthHemisphere);
void AddVertsForZCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForZCylinder3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForZCylinder3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices);
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices);
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& start, const Vec3& end, float coneHeight, float radius, int numSlices, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float interpolate, float cylinderRadius, float coneRadius, const Rgba8& coneColor, const Rgba8& cylinderColor, const AABB2& UVs, int numCylinderSlices, int numconeSlices);
void AddVertsForZCapsule3D(std::vector<Vertex_PCU>& verts, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numSlices);
void AddVertsForZCapsule3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices);
void AddVertsForCapsule3D(std::vector<Vertex_PCU>& verts, const Mat44& transform, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numSlices);
void AddVertsForCapsule3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Mat44& transform, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numSlices);
void AddVertsForPlane3D(std::vector<Vertex_PCU>& verts, const Plane3D& plane, int halfIterations);
void AddVertsForPlane3D(std::vector<Vertex_PCU>& verts, const Plane3D& plane, float distance, const Rgba8& color, const AABB2& UVs);
void AddVertsForPrism(std::vector<Vertex_PCU>& verts, const Vec3& baseCenter, float baseLength, float baseWidth, float prismHeight, Rgba8 const& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForPrism(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& baseCenter, float baseLength, float baseWidth, float prismHeight, Rgba8 const& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForPyramid(std::vector<Vertex_PCU>& verts, const Vec3& baseCenter, float baseLength, float baseHeight, float pyramidHeight, Rgba8 const& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForPyramid(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& baseCenter, float baseLength, float baseHeight, float pyramidHeight, Rgba8 const& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);