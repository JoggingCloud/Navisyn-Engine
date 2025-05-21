#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

void CalculateTangentSpaceBasisVectors(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, bool computeNormals /*= true*/, bool computeTangents /*= true*/)
{
	for (size_t i = 0; i < indexes.size(); i += 3)
	{
		unsigned int i0 = indexes[i];
		unsigned int i1 = indexes[i + 1];
		unsigned int i2 = indexes[i + 2];

		// Vertex pos
		Vec3& p0 = vertexes[i0].m_position;
		Vec3& p1 = vertexes[i1].m_position;
		Vec3& p2 = vertexes[i2].m_position;

		// Texture coordinates
		Vec2& uv0 = vertexes[i0].m_uvTexCoords;
		Vec2& uv1 = vertexes[i1].m_uvTexCoords;
		Vec2& uv2 = vertexes[i2].m_uvTexCoords;

		// Edge calculation
		Vec3 e0 = p1 - p0;
		Vec3 e1 = p2 - p0;

		if (computeNormals)
		{
			Vec3 N = CrossProduct3D(e0, e1);
			N.Normalize();

			vertexes[i0].m_normal += N;
			vertexes[i1].m_normal += N;
			vertexes[i2].m_normal += N;
		}

		if (computeTangents)
		{
			float deltaU0 = uv1.x - uv0.x;
			float deltaU1 = uv2.x - uv0.x;
			
			float deltaV0 = uv1.y - uv0.y;
			float deltaV1 = uv2.y - uv0.y;
		
			float r = 1.f / ((deltaU0 * deltaV1) - (deltaU1 * deltaV0));
			
			Vec3 tangent = r * ((deltaV1 * e0) - (deltaV0 * e1));
			Vec3 bitangent = r * ((deltaU0 * e1) - (deltaU1 * e0));

			vertexes[i0].m_tangent += tangent.GetNormalized();
			vertexes[i1].m_tangent += tangent.GetNormalized();
			vertexes[i2].m_tangent += tangent.GetNormalized();

			vertexes[i0].m_bitangent += bitangent.GetNormalized();
			vertexes[i1].m_bitangent += bitangent.GetNormalized();
			vertexes[i2].m_bitangent += bitangent.GetNormalized();
		}
	}

	for (size_t i = 0; i < vertexes.size(); i++)
	{
		vertexes[i].m_normal.Normalize();
		vertexes[i].m_tangent.Normalize();
		vertexes[i].m_bitangent.Normalize();

		// Gram-Schmidt orthonormalization
		float dP = DotProduct3D(vertexes[i].m_normal, vertexes[i].m_tangent);
		vertexes[i].m_tangent = vertexes[i].m_tangent - vertexes[i].m_normal * dP;
		vertexes[i].m_tangent.Normalize();

		if (DotProduct3D(CrossProduct3D(vertexes[i].m_normal, vertexes[i].m_tangent), vertexes[i].m_bitangent) < 0.f)
		{
			vertexes[i].m_tangent *= -1.f;
		}
	}
}

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, uniformScaleXY, rotationDegreesAboutZ, translationXY);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform)
{
	for (int index = 0; index < verts.size(); index++)
	{
		verts[index].m_position = transform.TransformPosition3D(verts[index].m_position);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform, bool hasNormals)
{
	if (hasNormals)
	{
		for (int index = 0; index < verts.size(); index++)
		{
			verts[index].m_position = transform.TransformPosition3D(verts[index].m_position);
			verts[index].m_normal = transform.TransformVectorQuantity3D(verts[index].m_normal);
			verts[index].m_tangent = transform.TransformVectorQuantity3D(verts[index].m_tangent);
			verts[index].m_bitangent = transform.TransformVectorQuantity3D(verts[index].m_bitangent);
		}
	}
	else 
	{
		for (int index = 0; index < verts.size(); index++)
		{
			verts[index].m_position = transform.TransformPosition3D(verts[index].m_position);
		}
	}
}

AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts)
{
	AABB2 bounds;
	for (const Vertex_PCU& vertex : verts)
	{
		Vec2 point;
		point.x = vertex.m_position.x;
		point.y = vertex.m_position.y;
		bounds.StretchToIncludePoint(point);
	}

	return bounds;
}

AABB3 GetVertexBounds3D(const std::vector<Vertex_PCU>& verts)
{
	AABB3 bounds;
	for (const Vertex_PCU& vertex : verts)
	{
		Vec3 point;
		point.x = vertex.m_position.x;
		point.y = vertex.m_position.y;
		point.z = vertex.m_position.z;
		bounds.StretchToIncludePoint(point);
	}

	return bounds;
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	const int numSegments = 32; // Number of segments for the half-circles
	const float angleIncrement = 360.0f / static_cast<float>(numSegments);

	Vec2 direction = (boneEnd - boneStart).GetNormalized();
	Vec2 perpendicular = direction.GetRotated90Degrees();

	for (int i = 0; i < numSegments; i++) 
	{
		float angle = static_cast<float>(i) * angleIncrement;
		Vec2 offset = perpendicular * radius * CosDegrees(angle);

		// Create instances of Vertex_PCU using the constructor
		Vertex_PCU vertex1(Vec3(boneStart.x + offset.x, boneStart.y + offset.y, 0.0f), color, Vec2(0.0f, 0.0f));
		Vertex_PCU vertex2(Vec3(boneEnd.x - offset.x, boneEnd.y - offset.y, 0.0f), color, Vec2(0.0f, 0.0f));

		// Add the vertices to the vector
		verts.emplace_back(vertex1);
		verts.emplace_back(vertex2);
	}

	// Create vertices for the central "bone" of the capsule
	Vertex_PCU vertex3(Vec3(boneStart.x, boneStart.y, 0.0f), color, Vec2(0.0f, 0.0f));
	Vertex_PCU vertex4(Vec3(boneEnd.x, boneEnd.y, 0.0f), color, Vec2(0.0f, 0.0f));

	// Add the vertices for the central "bone" to the vector
	verts.emplace_back(vertex3);
	verts.emplace_back(vertex4);

	// Create additional vertices to form the central "bone" as two triangles
	verts.emplace_back(vertex4);
	verts.emplace_back(vertex3);
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color)
{
	// Extract capsule properties
	Vec2 capsuleStart = capsule.m_start;
	Vec2 capsuleEnd = capsule.m_end;
	float radius = capsule.m_radius;

	// Calculate the number of segments for the half-circles
	const int numSegments = 30;
	const float angleIncrement = 360.0f / static_cast<float>(numSegments);

	Vec2 boneDirection = capsuleEnd - capsuleStart;
	float boneHalfLength = boneDirection.GetLength() * 0.5f;
	Vec2 boneNormal = boneDirection.GetNormalized();
	Vec2 capsuleCenter = 0.5f * (capsuleStart + capsuleEnd);

	// Calculate the perpendicular direction (90 degrees counterclockwise)
	Vec2 perpendicular = boneNormal.GetRotatedMinus90Degrees();
	float orientation = perpendicular.GetOrientationDegrees();

	for (int i = 0; i < numSegments; i++)
	{
		float angle1 = orientation + static_cast<float>(i) * angleIncrement;
		float angle2 = orientation + static_cast<float>(i + 1) * angleIncrement;

		if (angle2 <= 180.f + orientation)
		{
			Vertex_PCU vertex;

			// Vertex 1
			vertex.m_position = Vec3(capsuleEnd.x, capsuleEnd.y, 0.0f);
			vertex.m_color = color;
			verts.emplace_back(vertex);

			// Vertex 2
			vertex.m_position = Vec3(capsuleEnd.x + radius * CosDegrees(angle1), capsuleEnd.y + radius * SinDegrees(angle1), 0.0f);
			vertex.m_color = color;
			verts.emplace_back(vertex);

			// Vertex 3
			vertex.m_position = Vec3(capsuleEnd.x + radius * CosDegrees(angle2), capsuleEnd.y + radius * SinDegrees(angle2), 0.0f);
			vertex.m_color = color;
			verts.emplace_back(vertex);
		}

		if (angle1 >= 180.f + orientation)
		{
			Vertex_PCU vertex;

			// Vertex 1
			vertex.m_position = Vec3(capsuleStart.x, capsuleStart.y, 0.0f);
			vertex.m_color = color;
			verts.emplace_back(vertex);

			// Vertex 2
			vertex.m_position = Vec3(capsuleStart.x + radius * CosDegrees(angle1), capsuleStart.y + radius * SinDegrees(angle1), 0.0f);
			vertex.m_color = color;
			verts.emplace_back(vertex);

			// Vertex 3
			vertex.m_position = Vec3(capsuleStart.x + radius * CosDegrees(angle2), capsuleStart.y + radius * SinDegrees(angle2), 0.0f);
			vertex.m_color = color;
			verts.emplace_back(vertex);
		}
	}

	OBB2 square;
	square.m_center = capsuleCenter;
	square.m_halfDimensions = Vec2(radius, boneHalfLength);
	square.m_iBasisNormal = perpendicular;

	AddVertsForOBB2D(verts, square, color);
}

void AddVertsFor3DTriangle(std::vector<Vertex_PCU>& verts, Vec3 const& startPos, float radius, Rgba8 const& color)
{
	Vertex_PCU vertex;
	
	vertex.m_position = startPos;
	vertex.m_color = color;
	verts.emplace_back(vertex);

	vertex.m_position = Vec3(startPos.x + radius, startPos.y - (radius * 0.5f), startPos.z);
	vertex.m_color = color;						 
	verts.emplace_back(vertex);					 
												 
	vertex.m_position = Vec3(startPos.x + radius, startPos.y + (radius * 0.5f), startPos.z);
	vertex.m_color = color;
	verts.emplace_back(vertex);
}

void AddVertsFor3DTriangle(std::vector<Vertex_PCU>& verts, Vec3 const& startPos, Vec3 const& direction, float radius, Rgba8 const& color)
{
	Vertex_PCU vertex;

	// Normalize the direction vector
	Vec3 normalizedDirection = direction.GetNormalized();

	// Calculate the perpendicular vectors for the base of the triangle
	Vec3 rightOffset = Vec3(-normalizedDirection.y, normalizedDirection.x, 0.f) * (radius * 0.5f);
	Vec3 leftOffset = Vec3(normalizedDirection.y, -normalizedDirection.x, 0.f) * (radius * 0.5f);
	Vec3 baseOffset = normalizedDirection * radius; // Positive X direction

	// Tip vertex
	vertex.m_position = startPos;
	vertex.m_color = color;
	verts.emplace_back(vertex);

	// Base right vertex
	vertex.m_position = startPos + rightOffset - baseOffset;
	vertex.m_color = color;
	verts.emplace_back(vertex);

	// Base left vertex
	vertex.m_position = startPos + leftOffset - baseOffset;
	vertex.m_color = color;
	verts.emplace_back(vertex);
}

void AddVertsFor3DTriangle(std::vector<Vertex_PCU>& verts, Vec3 const& position1, Vec3 const& position2, Vec3 const& position3, Rgba8 const& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vertex_PCU vertex;

	vertex.m_position = position1;
	vertex.m_color = color;
	vertex.m_uvTexCoords = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	verts.emplace_back(vertex);

	vertex.m_position = position2;
	vertex.m_color = color;
	vertex.m_uvTexCoords = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	verts.emplace_back(vertex);

	vertex.m_position = position3;
	vertex.m_color = color;
	vertex.m_uvTexCoords = Vec2((UVs.m_mins.x + UVs.m_maxs.x) / 2, UVs.m_maxs.y);
	verts.emplace_back(vertex);
}

void AddVertsFor3DTriangle(std::vector<Vertex_PCUTBN>& verts,  std::vector<unsigned int>& indexes, Vec3 const& position1, Vec3 const& position2, Vec3 const& position3, Rgba8 const& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vertex_PCUTBN vertex;

	Vec3 edge1 = position2 - position1;
	Vec3 edge2 = position3 - position1;
	Vec3 normal = CrossProduct3D(edge1, edge2).GetNormalized();

	vertex.m_position = position1;
	vertex.m_normal = normal;
	vertex.m_color = color;
	vertex.m_uvTexCoords = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	verts.emplace_back(vertex);
	unsigned int index1 = static_cast<unsigned int>(verts.size() - 1);

	vertex.m_position = position2;
	vertex.m_normal = normal;
	vertex.m_color = color;
	vertex.m_uvTexCoords = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	verts.emplace_back(vertex);
	unsigned int index2 = static_cast<unsigned int>(verts.size() - 1);

	vertex.m_position = position3;
	vertex.m_normal = normal;
	vertex.m_color = color;
	vertex.m_uvTexCoords = Vec2((UVs.m_mins.x + UVs.m_maxs.x) / 2, UVs.m_maxs.y);
	verts.emplace_back(vertex);
	unsigned int index3 = static_cast<unsigned int>(verts.size() - 1);

	indexes.emplace_back(index1);
	indexes.emplace_back(index2);
	indexes.emplace_back(index3);

}

void AddVertsFor3DLinedTriangle(std::vector<Vertex_PCU>& verts, const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3, float lineThickness, Rgba8 const& color)
{
	LineSegment3 lineSegment1(vertex1, vertex2);
	LineSegment3 lineSegment2(vertex2, vertex3);
	LineSegment3 lineSegment3(vertex3, vertex1);

	AddVertsForLineSegmentOBB3D(verts, lineSegment1, lineThickness, color);
	AddVertsForLineSegmentOBB3D(verts, lineSegment2, lineThickness, color);
	AddVertsForLineSegmentOBB3D(verts, lineSegment3, lineThickness, color);
}

void AddVertsFor3DLinedTriangle(std::vector<Vertex_PCU>& verts, const LineSegment3& lineSegment1, const LineSegment3& lineSegment2, const LineSegment3& lineSegment3, float lineThickness, Rgba8 const& color)
{
	AddVertsForLineSegmentOBB3D(verts, lineSegment1, lineThickness, color);
	AddVertsForLineSegmentOBB3D(verts, lineSegment2, lineThickness, color);
	AddVertsForLineSegmentOBB3D(verts, lineSegment3, lineThickness, color);
}

void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, int sides, Rgba8 const& color)
{
	int NUM_DISC_VERTS = 3 * sides;
	float thetaDegrees = 360.f / static_cast<float>(sides);

	for (int sideNum = 0; sideNum < NUM_DISC_VERTS; sideNum++)
	{
		float thetaOne = sideNum * thetaDegrees;
		float thetaTwo = (sideNum + 1) * thetaDegrees;

		verts.emplace_back(Vertex_PCU(Vec3(center.x, center.y, 0.f), color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(Vec3(center.x + radius * CosDegrees(thetaOne), center.y + radius * SinDegrees(thetaOne), 0.f), color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(Vec3(center.x + radius * CosDegrees(thetaTwo), center.y + radius * SinDegrees(thetaTwo), 0.f), color, Vec2::ZERO));
	}
}

void AddVertsForDisc3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color)
{
	int NUM_DISC_SIDES = 64;
	int NUM_DISC_TRIS = NUM_DISC_SIDES;
	int NUM_DISC_VERTS = 3 * NUM_DISC_TRIS;
	float thetaDegrees = 360.f / static_cast<float>(NUM_DISC_VERTS);

	for (int sideNum = 0; sideNum < NUM_DISC_VERTS; sideNum++)
	{
		float thetaOne = sideNum * thetaDegrees;
		float thetaTwo = (sideNum + 1) * thetaDegrees;

		Vertex_PCU vertex;

		// Vertex 1
		vertex.m_position = center;
		vertex.m_color = color;
		verts.emplace_back(vertex);

		// Vertex 2
		vertex.m_position = Vec3(center.x + radius * CosDegrees(thetaOne), center.y + radius * SinDegrees(thetaOne), center.z);
		vertex.m_color = color;
		verts.emplace_back(vertex);

		// Vertex 3
		vertex.m_position = Vec3(center.x + radius * CosDegrees(thetaTwo), center.y + radius * SinDegrees(thetaTwo), center.z);
		vertex.m_color = color;
		verts.emplace_back(vertex);
	}
}

void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, int sides, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;

	int NUM_SIDES = sides;
	int NUM_TRIS = 2 * NUM_SIDES;
	int NUM_VERTS = 3 * NUM_TRIS;
	float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);

	int initialSize = static_cast<int>(verts.size());
	verts.resize(initialSize + NUM_VERTS);

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * (sideNum + 1);

		float cosStart = CosDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);

		float sinStart = SinDegrees(startDegrees);
		float sinEnd = SinDegrees(endDegrees);

		// Computing the inner and out length of the triangles 
		Vec3 innerStartPos = Vec3(center.x + (innerRadius * cosStart), center.y + (innerRadius * sinStart), 0.f);
		Vec3 innerEndPos = Vec3(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);

		Vec3 outerStartPos = Vec3(center.x + (outerRadius * cosStart), center.y + (outerRadius * sinStart), 0.f);
		Vec3 outerEndPos = Vec3(center.x + (outerRadius * cosEnd), center.y + (outerRadius * sinEnd), 0.f);

		int vertIndexA = initialSize + 6 * sideNum;
		int vertIndexB = vertIndexA + 1;
		int vertIndexC = vertIndexB + 1;
		int	vertIndexD = vertIndexC + 1;
		int	vertIndexE = vertIndexD + 1;
		int	vertIndexF = vertIndexE + 1;

		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexA].m_color = color;

		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexB].m_color = color;

		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexC].m_color = color;

		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexD].m_color = color;

		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexE].m_color = color;

		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexF].m_color = color;
	}
}

void AddVertsFor3DRing(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, int sides, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;

	int NUM_SIDES = sides;
	int NUM_TRIS = 2 * NUM_SIDES;
	int NUM_VERTS = 3 * NUM_TRIS;
	float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);

	verts.resize(NUM_VERTS);

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * (sideNum + 1);

		float cosStart = CosDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);

		float sinStart = SinDegrees(startDegrees);
		float sinEnd = SinDegrees(endDegrees);

		// Computing the inner and out length of the triangles 
		Vec3 innerStartPos = Vec3(center.x + (innerRadius * cosStart), center.y + (innerRadius * sinStart), center.z);
		Vec3 innerEndPos = Vec3(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, center.z);

		Vec3 outerStartPos = Vec3(center.x + (outerRadius * cosStart), center.y + (outerRadius * sinStart), center.z);
		Vec3 outerEndPos = Vec3(center.x + (outerRadius * cosEnd), center.y + (outerRadius * sinEnd), center.z);

		// Trapezoid is made of two triangles; ABC & DEF
		// A is innner end; B is inner Start; C is outer start
		// D is inner end; E is out start; F is outer end 
		int vertIndexA = 6 * sideNum;
		int vertIndexB = vertIndexA + 1;
		int vertIndexC = vertIndexB + 1;
		int	vertIndexD = vertIndexC + 1;
		int	vertIndexE = vertIndexD + 1;
		int	vertIndexF = vertIndexE + 1;

		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexA].m_color = color;
		
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexB].m_color = color;
		
		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexC].m_color = color;

		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexD].m_color = color;
		
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexE].m_color = color;
		
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexF].m_color = color;
	}
}

void AddVertsForHexagon3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float width, float thickness, Rgba8 const& color)
{
	float sideLength = (width * 0.5f) * 1.1547f;

	for (int i = 0; i < 6; i++)
	{
		float theta1 = 60.0f * i;
		float theta2 = 60.0f * (i + 1);

		Vec3 v1;
		Vec3 v2;

		v1.x = center.x + (sideLength - (thickness * 0.5f)) * CosDegrees(theta1);
		v1.y = center.y + (sideLength - (thickness * 0.5f)) * SinDegrees(theta1);
		v1.z = center.z;

		v2.x = center.x + (sideLength - (thickness * 0.5f)) * CosDegrees(theta2);
		v2.y = center.y + (sideLength - (thickness * 0.5f)) * SinDegrees(theta2);
		v2.z = center.z;

		verts.emplace_back(Vertex_PCU(center, color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(v1, color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(v2, color, Vec2::ZERO));
	}
}

void AddVertsForHollowHexagon3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float width, float thickness, Rgba8 const& color)
{
	float sideLength = (width * 0.5f) * 1.1547f;

	for (int i = 0; i < 6; i++)
	{
		float theta1 = 60.0f * i;
		float theta2 = 60.0f * (i + 1);

		Vec3 bl;
		Vec3 br;
		Vec3 tr;
		Vec3 tl;

		bl.x = center.x + (sideLength - (thickness * 0.5f)) * CosDegrees(theta1);
		bl.y = center.y + (sideLength - (thickness * 0.5f)) * SinDegrees(theta1);
		bl.z = center.z;

		br.x = center.x + (sideLength + (thickness * 0.5f)) * CosDegrees(theta1);
		br.y = center.y + (sideLength + (thickness * 0.5f)) * SinDegrees(theta1);
		br.z = center.z;

		tr.x = center.x + (sideLength + (thickness * 0.5f)) * CosDegrees(theta2);
		tr.y = center.y + (sideLength + (thickness * 0.5f)) * SinDegrees(theta2);
		tr.z = center.z;

		tl.x = center.x + (sideLength - (thickness * 0.5f)) * CosDegrees(theta2);
		tl.y = center.y + (sideLength - (thickness * 0.5f)) * SinDegrees(theta2);
		tl.z = center.z;

		verts.emplace_back(Vertex_PCU(bl, color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(br, color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(tr, color, Vec2::ZERO));

		verts.emplace_back(Vertex_PCU(tr, color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(tl, color, Vec2::ZERO));
		verts.emplace_back(Vertex_PCU(bl, color, Vec2::ZERO));
	}
}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
{
	// Define the vertices
	Vertex_PCU bottomLeftVertex(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f), color, Vec2(uvAtMins.x, uvAtMins.y));
	Vertex_PCU bottomRightVertex(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.0f), color, Vec2(uvAtMaxs.x, uvAtMins.y));
	Vertex_PCU topRightVertex(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f), color, Vec2(uvAtMaxs.x, uvAtMaxs.y));
	Vertex_PCU topLeftVertex(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.0f), color, Vec2(uvAtMins.x, uvAtMaxs.y));

	verts.emplace_back(bottomLeftVertex);
	verts.emplace_back(bottomRightVertex);
	verts.emplace_back(topRightVertex);
	verts.emplace_back(bottomLeftVertex);
	verts.emplace_back(topRightVertex);
	verts.emplace_back(topLeftVertex);
}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, const AABB2& bounds, float lineThickness, const Rgba8& color /*= Rgba8::WHITE*/)
{
	Vec2 bottomLeft = bounds.m_mins;
	Vec2 bottomRight = Vec2(bounds.m_maxs.x, bounds.m_mins.y);
	Vec2 topLeft = Vec2(bounds.m_mins.x, bounds.m_maxs.y);
	Vec2 topRight = bounds.m_maxs;

	AddVertsForLineSegment2D(verts, bottomLeft, bottomRight, lineThickness, color); // Bottom
	AddVertsForLineSegment2D(verts, bottomRight, topRight, lineThickness, color);   // Right
	AddVertsForLineSegment2D(verts, topRight, topLeft, lineThickness, color);       // Top
	AddVertsForLineSegment2D(verts, topLeft, bottomLeft, lineThickness, color);     // Left
}

void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color)
{
	// Extract properties from the OBB2 header file
	Vec2 center = box.m_center;
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	Vec2 halfDimensions = box.m_halfDimensions;

	// Calculate local vertices
	Vec2 localVertices[4];
	localVertices[0] = -iBasis * halfDimensions.x - jBasis * halfDimensions.y + center;
	localVertices[1] = iBasis * halfDimensions.x - jBasis * halfDimensions.y + center;
	localVertices[2] = iBasis * halfDimensions.x + jBasis * halfDimensions.y + center;
	localVertices[3] = -iBasis * halfDimensions.x + jBasis * halfDimensions.y + center;

	Vertex_PCU bottomLeft;
	bottomLeft.m_position = Vec3(localVertices[0]);
	bottomLeft.m_color = color;

	Vertex_PCU bottomRight;
	bottomRight.m_position = Vec3(localVertices[1]);
	bottomRight.m_color = color;


	Vertex_PCU TopRight;
	TopRight.m_position = Vec3(localVertices[2]);
	TopRight.m_color = color;

	Vertex_PCU TopLeft;
	TopLeft.m_position = Vec3(localVertices[3]);
	TopLeft.m_color = color;

	verts.emplace_back(TopLeft);
	verts.emplace_back(bottomLeft);
	verts.emplace_back(bottomRight);
	verts.emplace_back(TopRight);
	verts.emplace_back(TopLeft);
	verts.emplace_back(bottomRight);
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	// Calculate the direction of the line segment
	Vec2 direction = end - start;
	direction.Normalize();

	// Calculate the perpendicular direction (90 degrees counter-clockwise)
	Vec2 perpendicular = direction.GetRotated90Degrees();

	// Calculate half thickness
	float halfThickness = 0.5f * thickness;

	// Calculate the four corners of the rectangle
	Vec2 topLeft = start - perpendicular * halfThickness;
	Vec2 topRight = start + perpendicular * halfThickness;
	Vec2 bottomLeft = end - perpendicular * halfThickness;
	Vec2 bottomRight = end + perpendicular * halfThickness;

	// Create vertices for the rectangle
	Vertex_PCU vertex1, vertex2, vertex3, vertex4;
	vertex1.m_position = Vec3(topLeft.x, topLeft.y, 0.0f);
	vertex2.m_position = Vec3(topRight.x, topRight.y, 0.0f);
	vertex3.m_position = Vec3(bottomRight.x, bottomRight.y, 0.0f);
	vertex4.m_position = Vec3(bottomLeft.x, bottomLeft.y, 0.0f);

	vertex1.m_color = color;
	vertex2.m_color = color;
	vertex3.m_color = color;
	vertex4.m_color = color;

	// Add the vertices to the vector
	verts.emplace_back(vertex4);
	verts.emplace_back(vertex3);
	verts.emplace_back(vertex2);

	verts.emplace_back(vertex2);
	verts.emplace_back(vertex1);
	verts.emplace_back(vertex4);
}

void AddvertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color)
{
	// Extract the start and end points from the LineSegment2 object
	Vec2 start = lineSegment.m_start;
	Vec2 end = lineSegment.m_end;

	// Calculate the direction of the line segment
	Vec2 direction = end - start;
	direction.Normalize();

	// Use the GetRotated90Degrees function to calculate the perpendicular direction
	Vec2 perpendicular = direction.GetRotated90Degrees();

	// Calculate half thickness
	float halfThickness = 0.5f * thickness;

	// Calculate the four corners of the rectangle
	Vec2 topLeft = start - perpendicular * halfThickness;
	Vec2 topRight = start + perpendicular * halfThickness;
	Vec2 bottomLeft = end - perpendicular * halfThickness;
	Vec2 bottomRight = end + perpendicular * halfThickness;

	// Create vertices for the rectangle
	Vertex_PCU vertex1, vertex2, vertex3, vertex4;
	vertex1.m_position = Vec3(topLeft.x, topLeft.y, 0.0f);
	vertex2.m_position = Vec3(topRight.x, topRight.y, 0.0f);
	vertex3.m_position = Vec3(bottomRight.x, bottomRight.y, 0.0f);
	vertex4.m_position = Vec3(bottomLeft.x, bottomLeft.y, 0.0f);

	vertex1.m_color = color;
	vertex2.m_color = color;
	vertex3.m_color = color;
	vertex4.m_color = color;

	// Add the vertices to the vector
	verts.emplace_back(vertex4);
	verts.emplace_back(vertex3);
	verts.emplace_back(vertex2);

	verts.emplace_back(vertex2);
	verts.emplace_back(vertex1);
	verts.emplace_back(vertex4);
}

void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float lineThickness, Rgba8 const& color)
{
	// Calculate direction vector from start to end
	Vec3 direction = (end - start).GetNormalized();

	// Find a vector perpendicular to the direction for "thickness"
	Vec3 worldUp(0.f, 0.f, 1.f);
	Vec3 right = CrossProduct3D(direction, worldUp).GetNormalized() * (lineThickness * 0.5f);

	// Define the 8 corners of the cuboid
	Vec3 p0 = start - right;
	Vec3 p1 = start + right;
	Vec3 p2 = end + right;
	Vec3 p3 = end - right;

	// Now offset by the "up" vector for thickness along the Z axis
	Vec3 up = worldUp * (lineThickness * 0.5f);
	Vec3 p4 = p0 + up;
	Vec3 p5 = p1 + up;
	Vec3 p6 = p2 + up;
	Vec3 p7 = p3 + up;


	// Face 1 (p0, p1, p5, p4) - Bottom face
	verts.emplace_back(Vertex_PCU(p0, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p1, color, Vec2(1.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p5, color, Vec2(1.0f, 1.0f)));

	verts.emplace_back(Vertex_PCU(p0, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p5, color, Vec2(1.0f, 1.0f)));
	verts.emplace_back(Vertex_PCU(p4, color, Vec2(0.0f, 1.0f)));

	// Face 2 (p1, p2, p6, p5) - Right face
	verts.emplace_back(Vertex_PCU(p1, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p2, color, Vec2(1.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p6, color, Vec2(1.0f, 1.0f)));

	verts.emplace_back(Vertex_PCU(p1, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p6, color, Vec2(1.0f, 1.0f)));
	verts.emplace_back(Vertex_PCU(p5, color, Vec2(0.0f, 1.0f)));

	// Face 3 (p2, p3, p7, p6) - Top face
	verts.emplace_back(Vertex_PCU(p2, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p3, color, Vec2(1.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p7, color, Vec2(1.0f, 1.0f)));

	verts.emplace_back(Vertex_PCU(p2, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p7, color, Vec2(1.0f, 1.0f)));
	verts.emplace_back(Vertex_PCU(p6, color, Vec2(0.0f, 1.0f)));

	// Face 4 (p3, p0, p4, p7) - Left face
	verts.emplace_back(Vertex_PCU(p3, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p0, color, Vec2(1.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p4, color, Vec2(1.0f, 1.0f)));

	verts.emplace_back(Vertex_PCU(p3, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p4, color, Vec2(1.0f, 1.0f)));
	verts.emplace_back(Vertex_PCU(p7, color, Vec2(0.0f, 1.0f)));

	// Face 5 (p0, p3, p2, p1) - Start cap
	verts.emplace_back(Vertex_PCU(p0, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p3, color, Vec2(1.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p2, color, Vec2(1.0f, 1.0f)));

	verts.emplace_back(Vertex_PCU(p0, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p2, color, Vec2(1.0f, 1.0f)));
	verts.emplace_back(Vertex_PCU(p1, color, Vec2(0.0f, 1.0f)));

	// Face 6 (p4, p5, p6, p7) - End cap
	verts.emplace_back(Vertex_PCU(p4, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p5, color, Vec2(1.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p6, color, Vec2(1.0f, 1.0f)));

	verts.emplace_back(Vertex_PCU(p4, color, Vec2(0.0f, 0.0f)));
	verts.emplace_back(Vertex_PCU(p6, color, Vec2(1.0f, 1.0f)));
	verts.emplace_back(Vertex_PCU(p7, color, Vec2(0.0f, 1.0f)));
}

void AddvertsForLineSegmentAABB3D(std::vector<Vertex_PCU>& verts, const LineSegment3& lineSegment, float thickness, Rgba8 const& color)
{
	Vec3 start = lineSegment.m_start;
	Vec3 end = lineSegment.m_end;

	// Calculate half-thickness
	float halfThickness = thickness * 0.5f;

	// Calculate the minimum and maximum points for the AABB3
	Vec3 minPoint = Vec3(
		std::min(start.x, end.x) - halfThickness,
		std::min(start.y, end.y) - halfThickness,
		std::min(start.z, end.z) - halfThickness
	);

	Vec3 maxPoint = Vec3(
		std::max(start.x, end.x) + halfThickness,
		std::max(start.y, end.y) + halfThickness,
		std::max(start.z, end.z) + halfThickness
	);

	// Create AABB3 representing the line segment with thickness
	AABB3 aabb(start, end);
	AddVertsForAABB3D(verts, aabb, color);
}

void AddVertsForLineSegmentOBB3D(std::vector<Vertex_PCU>& verts, const LineSegment3& lineSegment, float thickness, Rgba8 const& color)
{
	Vec3 start = lineSegment.m_start;
	Vec3 end = lineSegment.m_end;

	// Calculate the normalized direction vector (kBasis)
	Vec3 kBasis = (end - start).GetNormalized();

	// Define a world up vector
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 worldForward = Vec3(1.0f, 0.0f, 0.0f);

	// Calculate perpendicular vectors (jBasis and iBasis)
	Vec3 jBasis = CrossProduct3D(worldUp, kBasis).GetNormalized();
	Vec3 iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	if (jBasis.GetLengthSquared() == 0.0f) 
	{
		// Handle edge case where kBasis is parallel to worldUp
		jBasis = CrossProduct3D(kBasis, worldForward).GetNormalized();
	}
	float length = (end - start).GetLength();

	Vec3 center = (start + end) * 0.5f;
	Vec3 halfExtents(length * 0.5f, thickness * 0.5f, thickness * 0.5f);
	OBB3 obb(center, iBasis, jBasis, kBasis, halfExtents);

	// Add vertices for the OBB3
	AddVertsForOBB3D(verts, obb, color);
}

void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	float length = GetDistance2D(tailPos, tipPos);

	// Calculate the direction of the vector
	Vec2 direction = (tipPos - tailPos) / length;
	Vec2 perpDirection = Vec2(-direction.y, direction.x);

	Vec2 arrowPoint1 = tipPos - (direction * arrowSize + perpDirection * arrowSize);
	Vec2 arrowPoint2 = tipPos - (direction * arrowSize - perpDirection * arrowSize);
	
	// Add verts for arrowhead
	verts.emplace_back(Vertex_PCU(Vec3(arrowPoint1.x, arrowPoint1.y, 0.0f), color, Vec2(0.f, 0.f)));
	verts.emplace_back(Vertex_PCU(Vec3(tipPos.x, tipPos.y, 0.0f), color, Vec2(0.f, 0.f)));
	verts.emplace_back(Vertex_PCU(Vec3(arrowPoint2.x, arrowPoint2.y, 0.0f), color, Vec2(0.f, 0.f)));

	// Use the AddVertsForLineSegment2D function for the arrow's body

	// Calculate the start position of the line segment
	Vec2 startSegment = tailPos + (direction * (length - arrowSize));

	// Create a LineSegment2 object representing the current line segment
	LineSegment2 lineSegment(startSegment, tailPos);

	// Add verts for the line segment 
	AddvertsForLineSegment2D(verts, lineSegment, lineThickness, color);
}

void AddVertsForCone2D(std::vector<Vertex_PCU>& verts, const Vec2& center, float radius, float coneAngleDegrees, float startAngleDegrees, const Rgba8& color)
{
	int NUM_CONE_SIDES = 32;
	int NUM_CONE_TRIS = NUM_CONE_SIDES;
	int NUM_CONE_VERTS = 3 * NUM_CONE_TRIS;
	float thetaDegrees = coneAngleDegrees / static_cast<float>(NUM_CONE_SIDES);

	for (int sideNum = 0; sideNum < NUM_CONE_VERTS; sideNum++)
	{
		float thetaOne = startAngleDegrees + sideNum * thetaDegrees;
		float thetaTwo = startAngleDegrees + (sideNum + 1) * thetaDegrees;

		Vertex_PCU vertex;

		// Vertex 1 (Center)
		vertex.m_position = Vec3(center.x, center.y, 0.0f);
		vertex.m_color = color;
		verts.emplace_back(vertex);

		// Vertex 2
		vertex.m_position = Vec3(center.x + radius * CosDegrees(thetaOne), center.y + radius * SinDegrees(thetaOne), 0.0f);
		vertex.m_color = color;
		verts.emplace_back(vertex);

		// Vertex 3
		vertex.m_position = Vec3(center.x + radius * CosDegrees(thetaTwo), center.y + radius * SinDegrees(thetaTwo), 0.0f);
		vertex.m_color = color;
		verts.emplace_back(vertex);
	}
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, float thickness, const Rgba8& color)
{
	LineSegment3 edge1(bottomLeft, bottomRight);
	LineSegment3 edge2(bottomRight, topRight);
	LineSegment3 edge3(topRight, topLeft);
	LineSegment3 edge4(topLeft, bottomLeft);

	AddvertsForLineSegmentAABB3D(verts, edge1, thickness, color);
	AddvertsForLineSegmentAABB3D(verts, edge2, thickness, color);
	AddvertsForLineSegmentAABB3D(verts, edge3, thickness, color);
	AddvertsForLineSegmentAABB3D(verts, edge4, thickness, color);
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vertex_PCU bottomLeftVertex = Vertex_PCU(bottomLeft, color, UVs.m_mins);
	Vertex_PCU bottomRightVertex = Vertex_PCU(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));

	Vertex_PCU TopRightVertex = Vertex_PCU(topRight, color, UVs.m_maxs);
	Vertex_PCU TopLeftVertex = Vertex_PCU(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));

	verts.emplace_back(bottomLeftVertex);
	verts.emplace_back(bottomRightVertex);
	verts.emplace_back(TopRightVertex);

	verts.emplace_back(TopRightVertex);
	verts.emplace_back(TopLeftVertex);
	verts.emplace_back(bottomLeftVertex);
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	// Record the current size of the verts vector to use as the starting index for the new vertices
	size_t startIndex = verts.size();

	// Create and append each vertex to the verts vector
	verts.emplace_back(bottomLeft, color, UVs.m_mins);
	verts.emplace_back(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));
	verts.emplace_back(topRight, color, UVs.m_maxs);
	verts.emplace_back(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));

	// First triangle (Bottom Left, Bottom Right, Top Right)
	indexes.emplace_back(static_cast<int>(startIndex));
	indexes.emplace_back(static_cast<int>(startIndex) + 1);
	indexes.emplace_back(static_cast<int>(startIndex) + 2);

	// Second triangle (Bottom Left, Top Right, Top Left)
	indexes.emplace_back(static_cast<int>(startIndex));
	indexes.emplace_back(static_cast<int>(startIndex) + 2);
	indexes.emplace_back(static_cast<int>(startIndex) + 3);
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	// Record the current size of the verts vector to use as the starting index for the new vertices
	size_t startIndex = verts.size();

	Vec3 edge1 = bottomRight - bottomLeft;
	Vec3 edge2 = topLeft - bottomLeft;

	Vec3 normal = CrossProduct3D(edge1, edge2).GetNormalized();
	Vec3 tangent = Vec3::ZERO;
	Vec3 bitangent = Vec3::ZERO;

	// Create and append each vertex to the verts vector
	verts.emplace_back(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal);
	verts.emplace_back(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, normal);
	verts.emplace_back(topRight, color, UVs.m_maxs, tangent, bitangent, normal);
	verts.emplace_back(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, normal);

	// First triangle (Bottom Left, Bottom Right, Top Right)
	indexes.emplace_back(static_cast<int>(startIndex));
	indexes.emplace_back(static_cast<int>(startIndex) + 1);
	indexes.emplace_back(static_cast<int>(startIndex) + 2);

	// Second triangle (Bottom Left, Top Right, Top Left)
	indexes.emplace_back(static_cast<int>(startIndex));
	indexes.emplace_back(static_cast<int>(startIndex) + 2);
	indexes.emplace_back(static_cast<int>(startIndex) + 3);
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 tangent = Vec3::ZERO;
	Vec3 bitangent = Vec3::ZERO;

	Vec3 edge1 = bottomRight - bottomLeft;
	Vec3 edge2 = topLeft - bottomLeft;

	Vec3 normal = CrossProduct3D(edge1, edge2).GetNormalized();

	Vertex_PCUTBN bottomLeftVertex = Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal);
	Vertex_PCUTBN bottomRightVertex = Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, normal);

	Vertex_PCUTBN TopRightVertex = Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, normal);
	Vertex_PCUTBN TopLeftVertex = Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, normal);

	verts.emplace_back(bottomLeftVertex);
	verts.emplace_back(bottomRightVertex);
	verts.emplace_back(TopRightVertex);

	verts.emplace_back(TopRightVertex);
	verts.emplace_back(TopLeftVertex);
	verts.emplace_back(bottomLeftVertex);
}

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	// Record the current size of the verts vector to use as the starting index for the new vertices
	size_t startIndex = verts.size();

	Vec3 tangent = Vec3::ZERO;
	Vec3 bitangent = Vec3::ZERO;

	// Calculate the middle points between top and bottom edges
	Vec3 centerBottomPoint = (bottomLeft + bottomRight) * 0.5f;
	Vec3 centerTopPoint = (topLeft + topRight) * 0.5f;

	// Calculate the vectors pointing from the middle points towards the respective corners
	Vec3 toCornerFromCenterBottomLeft = (bottomLeft - centerBottomPoint).GetNormalized();
	Vec3 toCornerFromCenterBottomRight = (bottomRight - centerBottomPoint).GetNormalized();
	Vec3 toCornerFromCenterTopLeft = (topLeft - centerTopPoint).GetNormalized();
	Vec3 toCornerFromCenterTopRight = (topRight - centerTopPoint).GetNormalized();

	// Calculate normals for the center bottom and center top by taking the cross product of the vectors towards the corners
	Vec3 centerBottomNormal = CrossProduct3D(toCornerFromCenterBottomRight, toCornerFromCenterBottomLeft).GetNormalized();
	Vec3 centerTopNormal = CrossProduct3D(toCornerFromCenterTopLeft, toCornerFromCenterTopRight).GetNormalized();

	// Create and append each vertex to the verts vector for quad 1
	verts.emplace_back(bottomLeft, color, UVs.m_mins, tangent, bitangent, toCornerFromCenterBottomLeft);
	verts.emplace_back(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, centerBottomNormal);
	verts.emplace_back(topRight, color, UVs.m_maxs, tangent, bitangent, centerTopNormal);
	verts.emplace_back(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, toCornerFromCenterTopLeft);

	// Create and append each vertex to the verts vector for quad 2
	verts.emplace_back(bottomLeft, color, UVs.m_mins, tangent, bitangent, centerBottomNormal);
	verts.emplace_back(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, toCornerFromCenterBottomRight);
	verts.emplace_back(topRight, color, UVs.m_maxs, tangent, bitangent, toCornerFromCenterTopRight);
	verts.emplace_back(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, centerTopNormal);

	// Quad 1
	indexes.emplace_back((int)startIndex);
	indexes.emplace_back((int)startIndex + 1);
	indexes.emplace_back((int)startIndex + 2);
	indexes.emplace_back((int)startIndex + 2);
	indexes.emplace_back((int)startIndex + 3);
	indexes.emplace_back((int)startIndex);

	// Quad 2
	indexes.emplace_back((int)startIndex + 1); // Bottom right of first quad (shared)
	indexes.emplace_back((int)startIndex + 2); // Top right of first quad (shared)
	indexes.emplace_back((int)startIndex + 4); // New vertex (bottom left of second quad)
	indexes.emplace_back((int)startIndex + 4); // New vertex (bottom left of second quad)
	indexes.emplace_back((int)startIndex + 5); // New vertex (top left of second quad)
	indexes.emplace_back((int)startIndex + 2); // Top right of first quad (shared)
}

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 tangent = Vec3::ZERO;
	Vec3 bitangent = Vec3::ZERO;

	// Calculate the middle points between top and bottom edges
	Vec3 centerBottomPoint = (bottomLeft + bottomRight) * 0.5f;
	Vec3 centerTopPoint = (topLeft + topRight) * 0.5f;

	// Calculate the vectors pointing from the middle points towards the respective corners
	Vec3 toCornerFromCenterBottomLeft = (bottomLeft - centerBottomPoint).GetNormalized();
	Vec3 toCornerFromCenterBottomRight = (bottomRight - centerBottomPoint).GetNormalized();
	Vec3 toCornerFromCenterTopLeft = (topLeft - centerTopPoint).GetNormalized();
	Vec3 toCornerFromCenterTopRight = (topRight - centerTopPoint).GetNormalized();

	// Calculate normals for the center bottom and center top by taking the cross product of the vectors towards the corners
	Vec3 centerBottomNormal = CrossProduct3D(toCornerFromCenterBottomRight, toCornerFromCenterBottomLeft).GetNormalized();
	Vec3 centerTopNormal = CrossProduct3D(toCornerFromCenterTopLeft, toCornerFromCenterTopRight).GetNormalized();

	// Create and append each vertex to the verts vector for quad 1
	verts.emplace_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, toCornerFromCenterBottomLeft));
	verts.emplace_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, centerBottomNormal));
	verts.emplace_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, centerTopNormal));

	verts.emplace_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, centerTopNormal));
	verts.emplace_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, toCornerFromCenterTopLeft));
	verts.emplace_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, toCornerFromCenterBottomLeft));

	// Create and append each vertex to the verts vector for quad 2
	verts.emplace_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, centerBottomNormal));
	verts.emplace_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, toCornerFromCenterBottomRight));
	verts.emplace_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, toCornerFromCenterTopRight));

	verts.emplace_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, toCornerFromCenterTopRight));
	verts.emplace_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, centerTopNormal));
	verts.emplace_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, centerBottomNormal));
}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	// Front face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Back face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Left face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Right face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Top face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Bottom face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // topLeft
		color, UVs);
}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, float lineThickness, const Rgba8& color /*= Rgba8::WHITE*/)
{
	// Front face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		lineThickness, color);

	// Back face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		lineThickness, color);

	// Left face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		lineThickness, color);

	// Right face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		lineThickness, color);

	// Top face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		lineThickness, color);

	// Bottom face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // topLeft
		lineThickness, color);
}

void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	// Front face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Back face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Left face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Right face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Top face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Bottom face
	AddVertsForQuad3D(verts,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // topLeft
		color, UVs);
}

void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	// Front face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Back face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Left face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Right face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Top face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Bottom face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // topLeft
		color, UVs);
}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	// Front face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Back face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Left face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Right face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // topLeft
		color, UVs);

	// Top face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),  // topLeft
		color, UVs);

	// Bottom face
	AddVertsForQuad3D(verts, indexes,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomLeft
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),  // bottomRight
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),  // topRight
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),  // topLeft
		color, UVs);
}

void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, const OBB3& box, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 center = box.m_center;
	Vec3 halfDimensions = box.m_halfDimensions;
	Vec3 iBasis = box.m_iBasisNormal;
	Vec3 jBasis = box.m_jBasisNormal;
	Vec3 kBasis = box.m_kBasisNormal;

	Vec3 bottomLeftBack = center - iBasis * halfDimensions.x + jBasis * halfDimensions.y - kBasis * halfDimensions.z;
	Vec3 bottomRightBack = center - iBasis * halfDimensions.x - jBasis * halfDimensions.y - kBasis * halfDimensions.z;
	Vec3 bottomLeftFront = center + iBasis * halfDimensions.x + jBasis * halfDimensions.y - kBasis * halfDimensions.z;
	Vec3 bottomRightFront = center + iBasis * halfDimensions.x - jBasis * halfDimensions.y - kBasis * halfDimensions.z;

	Vec3 topLeftBack = center - iBasis * halfDimensions.x + jBasis * halfDimensions.y + kBasis * halfDimensions.z;
	Vec3 topRightBack = center - iBasis * halfDimensions.x - jBasis * halfDimensions.y + kBasis * halfDimensions.z;
	Vec3 topLeftFront = center + iBasis * halfDimensions.x + jBasis * halfDimensions.y + kBasis * halfDimensions.z;
	Vec3 topRightFront = center + iBasis * halfDimensions.x - jBasis * halfDimensions.y + kBasis * halfDimensions.z;

	AddVertsForQuad3D(verts, bottomLeftBack, bottomRightBack, topRightBack, topLeftBack, color, UVs); // Back
	AddVertsForQuad3D(verts, bottomRightFront, bottomLeftFront, topLeftFront, topRightFront, color, UVs); // Front
	AddVertsForQuad3D(verts, bottomLeftFront, bottomLeftBack, topLeftBack, topLeftFront, color, UVs); // Left
	AddVertsForQuad3D(verts, bottomRightBack, bottomRightFront, topRightFront, topRightBack, color, UVs); // Right
	AddVertsForQuad3D(verts, topLeftBack, topRightBack, topRightFront, topLeftFront, color, UVs); // Top
	AddVertsForQuad3D(verts, bottomLeftFront, bottomRightFront, bottomRightBack, bottomLeftBack, color, UVs); // Bottom
}

void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const OBB3& box, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 center = box.m_center;
	Vec3 halfDimensions = box.m_halfDimensions;
	Vec3 iBasis = box.m_iBasisNormal;
	Vec3 jBasis = box.m_jBasisNormal;
	Vec3 kBasis = box.m_kBasisNormal;

	Vec3 bottomLeftBack = center - iBasis * halfDimensions.x + jBasis * halfDimensions.y - kBasis * halfDimensions.z;
	Vec3 bottomRightBack = center - iBasis * halfDimensions.x - jBasis * halfDimensions.y - kBasis * halfDimensions.z;
	Vec3 bottomLeftFront = center + iBasis * halfDimensions.x + jBasis * halfDimensions.y - kBasis * halfDimensions.z;
	Vec3 bottomRightFront = center + iBasis * halfDimensions.x - jBasis * halfDimensions.y - kBasis * halfDimensions.z;

	Vec3 topLeftBack = center - iBasis * halfDimensions.x + jBasis * halfDimensions.y + kBasis * halfDimensions.z;
	Vec3 topRightBack = center - iBasis * halfDimensions.x - jBasis * halfDimensions.y + kBasis * halfDimensions.z;
	Vec3 topLeftFront = center + iBasis * halfDimensions.x + jBasis * halfDimensions.y + kBasis * halfDimensions.z;
	Vec3 topRightFront = center + iBasis * halfDimensions.x - jBasis * halfDimensions.y + kBasis * halfDimensions.z;

	AddVertsForQuad3D(verts, indexes, bottomLeftBack, bottomRightBack, topRightBack, topLeftBack, color, UVs); // Back
	AddVertsForQuad3D(verts, indexes, bottomRightFront, bottomLeftFront, topLeftFront, topRightFront, color, UVs); // Front
	AddVertsForQuad3D(verts, indexes, bottomLeftFront, bottomLeftBack, topLeftBack, topLeftFront, color, UVs); // Left
	AddVertsForQuad3D(verts, indexes, bottomRightBack, bottomRightFront, topRightFront, topRightBack, color, UVs); // Right
	AddVertsForQuad3D(verts, indexes, topLeftBack, topRightBack, topRightFront, topLeftFront, color, UVs); // Top
	AddVertsForQuad3D(verts, indexes, bottomLeftFront, bottomRightFront, bottomRightBack, bottomLeftBack, color, UVs); // Bottom
}

void AddVertsForZSphere(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 180.f / (numLatitudeSlices - 1);

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices - 90.f; // north pole
			float longitude1 = longitude * angleBetweenLongitudeSlices;
			
			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices - 90.f; // south pole
			float longitude2 = (longitude + 1) * angleBetweenLongitudeSlices; 

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;
			
			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForZSphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.0f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 180.f / (numLatitudeSlices - 1);

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices - 90.f; // north pole
			float longitude1 = longitude * angleBetweenLongitudeSlices;

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices - 90.f; // south pole
			float longitude2 = (longitude + 1) * angleBetweenLongitudeSlices;

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, indexes, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForZSphere(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.0f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 180.f / (numLatitudeSlices - 1);

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices - 90.f; // north pole
			float longitude1 = longitude * angleBetweenLongitudeSlices;

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices - 90.f; // south pole
			float longitude2 = (longitude + 1) * angleBetweenLongitudeSlices;

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, indexes, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForZSphere(std::vector<Vertex_PCUTBN>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.0f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 180.f / (numLatitudeSlices - 1);

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices - 90.f; // north pole
			float longitude1 = longitude * angleBetweenLongitudeSlices;

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices - 90.f; // south pole
			float longitude2 = (longitude + 1) * angleBetweenLongitudeSlices;

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForZSphere(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.f / numLongitudeSlices;
	float angleBetweenLatitudeSlices = 180.f / numLatitudeSlices;

	// Calculate south pole tangent, bitangent, normal
	Vec3 southNormal(0.f, 0.f, -1.f);
	Vec3 southTangent(1.f, 0.f, 0.f);
	Vec3 southBitangent(0.f, 1.f, 0.f);

	// Add the bottom pole vertex (south pole)
	verts.emplace_back(Vertex_PCUTBN(center + Vec3(0.f, 0.f, -radius), color, Vec2(0.5f, 0.f), southTangent, southBitangent, southNormal));

	for (int latitude = 1; latitude < numLatitudeSlices; latitude++)
	{
		float latitudeAngle = latitude * angleBetweenLatitudeSlices - 90.f;
		float vTexCoord = RangeMapClamped((float)latitude, 0.f, (float)numLatitudeSlices, UVs.m_mins.y, UVs.m_maxs.y);

		for (int longitude = 0; longitude <= numLongitudeSlices; longitude++)
		{
			float longitudeAngle = longitude * angleBetweenLongitudeSlices;
			Vec3 position = Vec3::MakeFromPolarDegrees(latitudeAngle, longitudeAngle, radius) + center;
			Vec3 normal = position.GetNormalized();
			Vec3 tangent = Vec3::MakeFromPolarDegrees(0, longitudeAngle);
			Vec3 bitangent = CrossProduct3D(normal, tangent);

			float uTexCoord = RangeMapClamped((float)longitude, 0.f, (float)numLongitudeSlices, UVs.m_mins.x, UVs.m_maxs.x);
			verts.emplace_back(Vertex_PCUTBN(position, color, Vec2(uTexCoord, vTexCoord), tangent, bitangent, normal));
		}
	}

	// Calculate north pole tangent, bitangent, normal
	Vec3 northNormal(0.f, 0.f, 1.f);
	Vec3 northTangent(1.f, 0.f, 0.f);
	Vec3 northBitangent(0.f, 1.f, 0.f);

	// Add the top pole vertex (north pole)
	verts.emplace_back(Vertex_PCUTBN(center + Vec3(0.f, 0.f, radius), color, Vec2(0.5f, 1.0f), northTangent, northBitangent, northNormal));

	int numVerticesPerStack = numLongitudeSlices + 1;

	// Index for the south pole vertex
	int southPoleIndex = 0;
	// Index for the north pole vertex
	int northPoleIndex = static_cast<int>(verts.size()) - 1;

	// Bottom cap (south pole)
	for (int longitude = 0; longitude < numLongitudeSlices; ++longitude) 
	{
		// BL is the current vertex in the first ring
		int BL = 1 + longitude;

		// BR is the next vertex in the first ring
		int BR = 1 + (longitude + 1) % numLongitudeSlices;

		// Create the triangle with the south pole
		indexes.emplace_back(southPoleIndex);  // South pole
		indexes.emplace_back(BR);              // Bottom right (next vertex)
		indexes.emplace_back(BL);              // Bottom left (current vertex)
	}

	// Middle stacks
	for (int latitude = 0; latitude < numLatitudeSlices - 2; ++latitude) 
	{
		for (int longitude = 0; longitude < numLongitudeSlices; ++longitude) 
		{
			int BL = 1 + latitude * numVerticesPerStack + longitude;
			int BR = BL + 1;
			int TL = BL + numVerticesPerStack;
			int TR = TL + 1;

			// First triangle: BL, BR, TR
			indexes.emplace_back(BL);
			indexes.emplace_back(BR);
			indexes.emplace_back(TR);

			// Second triangle: BL, TR, TL
			indexes.emplace_back(BL);
			indexes.emplace_back(TR);
			indexes.emplace_back(TL);
		}
	}

	// Top cap (north pole)
	for (int longitude = 0; longitude < numLongitudeSlices; ++longitude) 
	{
		int lastStackStart = 1 + (numLatitudeSlices - 2) * numVerticesPerStack; // Last ring start

		// BL is the current vertex in the last latitude ring
		int BL = lastStackStart + longitude;

		// BR is the next vertex in the last latitude ring
		int BR = lastStackStart + (longitude + 1) % numLongitudeSlices;

		// Create the triangle with the north pole
		indexes.emplace_back(northPoleIndex);  // North pole
		indexes.emplace_back(BL);              // Bottom left (current vertex)
		indexes.emplace_back(BR);              // Bottom right (next vertex)
	}
}

void AddVertsForSphere(std::vector<Vertex_PCU>& verts, const Mat44& transform, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.0f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 180.f / (numLatitudeSlices - 1);

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices - 90.f; // north pole
			float longitude1 = longitude * angleBetweenLongitudeSlices;

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices - 90.f; // south pole
			float longitude2 = (longitude + 1) * angleBetweenLongitudeSlices;

			Vec3 bottomLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center);
			Vec3 bottomRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center);
			Vec3 topRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center);
			Vec3 topLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center);

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForSphere(std::vector<Vertex_PCU>& verts, const Mat44& transform, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.0f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 180.f / (numLatitudeSlices - 1);

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices - 90.f; // north pole
			float longitude1 = longitude * angleBetweenLongitudeSlices;

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices - 90.f; // south pole
			float longitude2 = (longitude + 1) * angleBetweenLongitudeSlices;

			Vec3 bottomLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center);
			Vec3 bottomRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center);
			Vec3 topRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center);
			Vec3 topLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center);

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, -90.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, indexes, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForNorthZHemisphere(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 90.f / (numLatitudeSlices - 1); // Only cover half the sphere

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices;
			float longitude1 = (longitude * angleBetweenLongitudeSlices);

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices;
			float longitude2 = ((longitude + 1) * angleBetweenLongitudeSlices);

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForNorthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = 90.f / numLatitudeSlices; // Only cover half the sphere

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices;
			float longitude1 = (longitude * angleBetweenLongitudeSlices);

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices;
			float longitude2 = ((longitude + 1) * angleBetweenLongitudeSlices);

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;
				
			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, indexes, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForNorthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices)
{
	float angleBetweenLongitudeSlices = 360.f / numLongitudeSlices;
	float angleBetweenLatitudeSlices = 90.f / numLatitudeSlices;

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude <= numLongitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices;
			float longitude1 = longitude * angleBetweenLongitudeSlices;

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices;
			float longitude2 = ((longitude + 1) * angleBetweenLongitudeSlices);

			// Position calculations
			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			// First triangle: Bottom left ? Bottom right ? Top right
			verts.emplace_back(Vertex_PCU(bottomLeft, color, Vec2(uMin, vMin)));
			verts.emplace_back(Vertex_PCU(bottomRight, color, Vec2(uMax, vMin)));
			verts.emplace_back(Vertex_PCU(topRight, color, Vec2(uMax, vMax)));

			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 3));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 2));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 1));

			// Second triangle: Bottom left ? Top right ? Top left
			verts.emplace_back(Vertex_PCU(bottomLeft, color, Vec2(uMin, vMin)));
			verts.emplace_back(Vertex_PCU(topRight, color, Vec2(uMax, vMax)));
			verts.emplace_back(Vertex_PCU(topLeft, color, Vec2(uMin, vMax)));

			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 3));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 2));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 1));
		}
	}
}

void AddVertsForSouthZHemisphere(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = -360.f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = -90.f / (numLatitudeSlices + 1); // Only cover half the sphere

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices;
			float longitude1 = (longitude * angleBetweenLongitudeSlices);

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices;
			float longitude2 = ((longitude + 1) * angleBetweenLongitudeSlices);

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, -360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, -360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, 0.f, -90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, 0.f, -90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForSouthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	float angleBetweenLongitudeSlices = -360.f / numLatitudeSlices;
	float angleBetweenLatitudeSlices = -90.f / numLatitudeSlices; // Only cover half the sphere

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices;
			float longitude1 = (longitude * angleBetweenLongitudeSlices);

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices;
			float longitude2 = ((longitude + 1) * angleBetweenLongitudeSlices);

			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, -360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, -360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, 0.f, -90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, 0.f, -90.f, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, indexes, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForSouthZHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices)
{
	float angleBetweenLongitudeSlices = -360.f / numLongitudeSlices;
	float angleBetweenLatitudeSlices = -90.f / numLatitudeSlices;

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude <= numLongitudeSlices; longitude++)
		{
			float latitude1 = latitude * angleBetweenLatitudeSlices;
			float longitude1 = longitude * angleBetweenLongitudeSlices;

			float latitude2 = (latitude + 1) * angleBetweenLatitudeSlices;
			float longitude2 = ((longitude + 1) * angleBetweenLongitudeSlices);

			// Position calculations
			Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center;
			Vec3 bottomRight = Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center;
			Vec3 topLeft = Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center;
			Vec3 topRight = Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center;

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			// First triangle: Bottom left ? Bottom right ? Top right
			verts.emplace_back(Vertex_PCU(bottomLeft, color, Vec2(uMin, vMin)));
			verts.emplace_back(Vertex_PCU(bottomRight, color, Vec2(uMax, vMin)));
			verts.emplace_back(Vertex_PCU(topRight, color, Vec2(uMax, vMax)));

			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 3));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 2));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 1));

			// Second triangle: Bottom left ? Top right ? Top left
			verts.emplace_back(Vertex_PCU(bottomLeft, color, Vec2(uMin, vMin)));
			verts.emplace_back(Vertex_PCU(topRight, color, Vec2(uMax, vMax)));
			verts.emplace_back(Vertex_PCU(topLeft, color, Vec2(uMin, vMax)));

			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 3));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 2));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 1));
		}
	}
}

void AddVertsForHemisphere(std::vector<Vertex_PCU>& verts, const Mat44& transform, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, bool isNorthHemisphere)
{
	float latitudeStart = isNorthHemisphere ? 0.f : -90.f;
	float latitudeEnd = isNorthHemisphere ? 90.f : 0.f;

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = RangeMapClamped((float)latitude, 0.f, (float)numLatitudeSlices, latitudeStart, latitudeEnd);
			float longitude1 = RangeMapClamped((float)longitude, 0.f, (float)numLatitudeSlices, 0.f, 360.f);

			float latitude2 = RangeMapClamped((float)latitude + 1.f, 0.f, (float)numLatitudeSlices, latitudeStart, latitudeEnd);
			float longitude2 = RangeMapClamped((float)longitude + 1.f, 0.f, (float)numLatitudeSlices, 0.f, 360.f);

			Vec3 bottomLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center);
			Vec3 bottomRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center);
			Vec3 topRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center);
			Vec3 topLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center);

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float vMin = RangeMapClamped(latitude1, latitudeStart, latitudeEnd, UVs.m_mins.y, UVs.m_maxs.y);
			float vMax = RangeMapClamped(latitude2, latitudeStart, latitudeEnd, UVs.m_mins.y, UVs.m_maxs.y);

			AABB2 adjustedUvs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

			AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, color, adjustedUvs);
		}
	}
}

void AddVertsForHemisphere(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Mat44& transform, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices, bool isNorthHemisphere)
{
	float latitudeStart = isNorthHemisphere ? 0.f : -90.f;
	float latitudeEnd = isNorthHemisphere ? 90.f : 0.f;

	for (int latitude = 0; latitude < numLatitudeSlices; latitude++)
	{
		for (int longitude = 0; longitude < numLatitudeSlices; longitude++)
		{
			float latitude1 = RangeMapClamped((float)latitude, 0.f, (float)numLatitudeSlices, latitudeStart, latitudeEnd);
			float longitude1 = RangeMapClamped((float)longitude, 0.f, (float)numLatitudeSlices, 0.f, 360.f);

			float latitude2 = RangeMapClamped((float)latitude + 1.f, 0.f, (float)numLatitudeSlices, latitudeStart, latitudeEnd);
			float longitude2 = RangeMapClamped((float)longitude + 1.f, 0.f, (float)numLatitudeSlices, 0.f, 360.f);

			Vec3 bottomLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude1, radius) + center);
			Vec3 bottomRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude1, longitude2, radius) + center);
			Vec3 topRight = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude2, radius) + center);
			Vec3 topLeft = transform.TransformPosition3D(Vec3::MakeFromPolarDegrees(latitude2, longitude1, radius) + center);

			// Calculate UVs based on latitude and longitude
			float uMin = RangeMapClamped(longitude1, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			float uMax = RangeMapClamped(longitude2, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);

			// Flip the v UVs for the south hemisphere
			float vMin, vMax;
			if (isNorthHemisphere) 
			{
				vMin = RangeMapClamped(latitude1, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
				vMax = RangeMapClamped(latitude2, 0.f, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			}
			else 
			{
				vMax = RangeMapClamped(latitude1, -90.f, 0.f, UVs.m_mins.y, UVs.m_maxs.y); // Swap min/max
				vMin = RangeMapClamped(latitude2, -90.f, 0.f, UVs.m_mins.y, UVs.m_maxs.y); // Swap min/max
			}

			// First triangle: Bottom left -> Bottom right -> Top right
			verts.emplace_back(Vertex_PCU(bottomLeft, color, Vec2(uMin, vMin)));
			verts.emplace_back(Vertex_PCU(bottomRight, color, Vec2(uMax, vMin)));
			verts.emplace_back(Vertex_PCU(topRight, color, Vec2(uMax, vMax)));

			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 3));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 2));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 1));

			// Second triangle: Bottom left -> Top right -> Top left
			verts.emplace_back(Vertex_PCU(bottomLeft, color, Vec2(uMin, vMin)));
			verts.emplace_back(Vertex_PCU(topRight, color, Vec2(uMax, vMax)));
			verts.emplace_back(Vertex_PCU(topLeft, color, Vec2(uMin, vMax)));

			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 3));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 2));
			indexes.emplace_back(static_cast<unsigned int>(verts.size() - 1));
		}
	}
}

void AddVertsForZCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& color, const AABB2& UVs/* = AABB2::ZERO_TO_ONE*/)
{
	int startIndex = static_cast<int>(verts.size());

	Vec3 Kbasis = (end - start).GetNormalized();
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 worldForward = Vec3(1.0f, 0.0f, 0.0f);
	
	Vec3 JBasis = CrossProduct3D(worldUp, Kbasis).GetNormalized();
	Vec3 IBasis = CrossProduct3D(JBasis, Kbasis);

	if (fabs(DotProduct3D(Kbasis, worldUp)) > 0.99f)
	{
		JBasis = CrossProduct3D(Kbasis, worldForward);
		IBasis = CrossProduct3D(JBasis, Kbasis);
	}

	float length = (end - start).GetLength();
	
	const float theta = 360.0f / static_cast<float>(numSlices);

	for (int i = 0; i < numSlices; i++)
	{
		Vertex_PCU vertex1, vertex2, vertex3, vertex4;

		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		vertex1.m_position = Vec3((CosDegrees(thetaCurrent) * radius), (SinDegrees(thetaCurrent) * radius), 0.f);
		vertex1.m_uvTexCoords = Vec2(uMin, vMin);
		vertex1.m_color = color;

		vertex2.m_position = Vec3((CosDegrees(thetaNext) * radius), (SinDegrees(thetaNext) * radius), 0.f);
		vertex2.m_uvTexCoords = Vec2(uMax, vMin);
		vertex2.m_color = color;

		vertex3.m_position = Vec3((CosDegrees(thetaCurrent) * radius), (SinDegrees(thetaCurrent) * radius), length);
		vertex3.m_uvTexCoords = Vec2(uMin, vMax);
		vertex3.m_color = color;

		vertex4.m_position = Vec3((CosDegrees(thetaNext) * radius), (SinDegrees(thetaNext) * radius), length);
		vertex4.m_uvTexCoords = Vec2(uMax, vMax);
		vertex4.m_color = color;

		verts.emplace_back(Vertex_PCU(Vec3(0.f, 0.f, 0.f), color, Vec2(UVs.GetCenter().x, UVs.GetCenter().x)));
		verts.emplace_back(vertex2);
		verts.emplace_back(vertex1);

		verts.emplace_back(Vertex_PCU(Vec3(0.f, 0.f, length), color, Vec2(UVs.GetCenter().x, UVs.GetCenter().x)));
		verts.emplace_back(vertex3);
		verts.emplace_back(vertex4);

		AABB2 correctedUVs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));
		AddVertsForQuad3D(verts, vertex1.m_position, vertex2.m_position, vertex4.m_position, vertex3.m_position, color, correctedUVs);
	}

	Mat44 matrix;	
	matrix.SetIJKT3D(IBasis.GetNormalized(), JBasis.GetNormalized(), Kbasis.GetNormalized(), start);
	
	for (int index = startIndex; index < verts.size(); index++)
	{
		verts[index].m_position = matrix.TransformPosition3D(verts[index].m_position);
	}
}

void AddVertsForZCylinder3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& color, const AABB2& UVs/* = AABB2::ZERO_TO_ONE*/)
{
	int startIndex = static_cast<int>(verts.size());

	Vec3 Kbasis = (end - start).GetNormalized();
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 worldForward = Vec3(1.0f, 0.0f, 0.0f);

	Vec3 JBasis = CrossProduct3D(worldUp, Kbasis).GetNormalized();
	Vec3 IBasis = CrossProduct3D(JBasis, Kbasis);

	if (fabs(DotProduct3D(Kbasis, worldUp)) > 0.99f)
	{
		JBasis = CrossProduct3D(Kbasis, worldForward);
		IBasis = CrossProduct3D(JBasis, Kbasis);
	}

	float length = (end - start).GetLength();
	const float theta = 360.0f / static_cast<float>(numSlices);

	Vertex_PCU bottomCenter = { Vec3(0.f, 0.f, 0.f), color, Vec2(UVs.GetCenter().x, UVs.GetCenter().y) };
	Vertex_PCU topCenter = { Vec3(0.f, 0.f, length), color, Vec2(UVs.GetCenter().x, UVs.GetCenter().y) };

	int bottomCenterIndex = static_cast<int>(verts.size());
	verts.emplace_back(bottomCenter);

	int topCenterIndex = static_cast<int>(verts.size());
	verts.emplace_back(topCenter);

	for (int i = 0; i < numSlices; i++)
	{
		Vertex_PCU vertex1, vertex2, vertex3, vertex4;

		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		vertex1.m_position = Vec3((CosDegrees(thetaCurrent) * radius), (SinDegrees(thetaCurrent) * radius), 0.f);
		vertex1.m_uvTexCoords = Vec2(uMin, vMin);
		vertex1.m_color = color;

		vertex2.m_position = Vec3((CosDegrees(thetaNext) * radius), (SinDegrees(thetaNext) * radius), 0.f);
		vertex2.m_uvTexCoords = Vec2(uMax, vMin);
		vertex2.m_color = color;

		vertex3.m_position = Vec3((CosDegrees(thetaCurrent) * radius), (SinDegrees(thetaCurrent) * radius), length);
		vertex3.m_uvTexCoords = Vec2(uMin, vMax);
		vertex3.m_color = color;

		vertex4.m_position = Vec3((CosDegrees(thetaNext) * radius), (SinDegrees(thetaNext) * radius), length);
		vertex4.m_uvTexCoords = Vec2(uMax, vMax);
		vertex4.m_color = color;

		AABB2 correctedUVs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));
		AddVertsForQuad3D(verts, indexes, vertex1.m_position, vertex2.m_position, vertex4.m_position, vertex3.m_position, color, correctedUVs);
	
		// Add bottom cap triangle
		indexes.emplace_back(bottomCenterIndex);
		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex2);

		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex1);

		// Add top cap triangle
		indexes.emplace_back(topCenterIndex);
		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex3);

		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex4);
	}

	Mat44 matrix;
	matrix.SetIJKT3D(IBasis.GetNormalized(), JBasis.GetNormalized(), Kbasis.GetNormalized(), start);

	for (int index = startIndex; index < verts.size(); index++)
	{
		verts[index].m_position = matrix.TransformPosition3D(verts[index].m_position);
	}
}

void AddVertsForZCylinder3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& color, const AABB2& UVs/* = AABB2::ZERO_TO_ONE*/)
{
	int startIndex = static_cast<int>(verts.size());

	Vec3 Kbasis = (end - start).GetNormalized();
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 worldForward = Vec3(1.0f, 0.0f, 0.0f);

	Vec3 JBasis = CrossProduct3D(worldUp, Kbasis).GetNormalized();
	Vec3 IBasis = CrossProduct3D(JBasis, Kbasis);

	if (fabs(DotProduct3D(Kbasis, worldUp)) > 0.99f)
	{
		JBasis = CrossProduct3D(Kbasis, worldForward);
		IBasis = CrossProduct3D(JBasis, Kbasis);
	}

	float length = (end - start).GetLength();
	const float theta = 360.0f / static_cast<float>(numSlices);

	Vertex_PCUTBN bottomCenter = { Vec3(0.f, 0.f, 0.f), color, Vec2(UVs.GetCenter().x, UVs.GetCenter().y) };
	Vertex_PCUTBN topCenter = { Vec3(0.f, 0.f, length), color, Vec2(UVs.GetCenter().x, UVs.GetCenter().y) };

	int bottomCenterIndex = static_cast<int>(verts.size());
	verts.emplace_back(bottomCenter);

	int topCenterIndex = static_cast<int>(verts.size());
	verts.emplace_back(topCenter);

	for (int i = 0; i < numSlices; i++)
	{
		Vertex_PCUTBN vertex1, vertex2, vertex3, vertex4;

		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float sinThetaCurrent = SinDegrees(thetaCurrent);
		float cosThetaCurrent = CosDegrees(thetaCurrent);
		float sinThetaNext = SinDegrees(thetaNext);
		float cosThetaNext = CosDegrees(thetaNext);

		float uMin = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		// Vertex 1
		vertex1.m_position = Vec3(cosThetaCurrent * radius, sinThetaCurrent * radius, 0.f);
		vertex1.m_normal = Vec3(cosThetaCurrent, sinThetaCurrent, 0.f).GetNormalized();
		vertex1.m_uvTexCoords = Vec2(uMin, vMin);
		vertex1.m_color = color;

		// Vertex 2
		vertex2.m_position = Vec3(cosThetaNext * radius, sinThetaNext * radius, 0.f);
		vertex2.m_normal = Vec3(cosThetaNext, sinThetaNext, 0.f).GetNormalized();
		vertex2.m_uvTexCoords = Vec2(uMax, vMin);
		vertex2.m_color = color;

		// Vertex 3
		vertex3.m_position = Vec3(cosThetaCurrent * radius, sinThetaCurrent * radius, length);
		vertex3.m_normal = Vec3(cosThetaCurrent, sinThetaCurrent, 0.f).GetNormalized();
		vertex3.m_uvTexCoords = Vec2(uMin, vMax);
		vertex3.m_color = color;

		// Vertex 4
		vertex4.m_position = Vec3(cosThetaNext * radius, sinThetaNext * radius, length);
		vertex4.m_normal = Vec3(cosThetaNext, sinThetaNext, 0.f).GetNormalized();
		vertex4.m_uvTexCoords = Vec2(uMax, vMax);
		vertex4.m_color = color;

		// Build the side quad
		AABB2 correctedUVs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));
		AddVertsForQuad3D(verts, indexes, vertex1.m_position, vertex2.m_position, vertex4.m_position, vertex3.m_position, color, correctedUVs);
	
		// Add bottom cap triangle
		indexes.emplace_back(bottomCenterIndex);
		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex2);

		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex1);

		// Add top cap triangle
		indexes.emplace_back(topCenterIndex);
		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex3);

		indexes.emplace_back(static_cast<unsigned int>(verts.size()));
		verts.emplace_back(vertex4);
	}

	// Transform vertices to align the cylinder along its axis
	Mat44 matrix;
	matrix.SetIJKT3D(IBasis.GetNormalized(), JBasis.GetNormalized(), Kbasis.GetNormalized(), start);

	for (int index = startIndex; index < verts.size(); index++)
	{
		verts[index].m_position = matrix.TransformPosition3D(verts[index].m_position);
	}
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices)
{
	int startIndex = static_cast<int>(verts.size());

	Vec3 axis = (end - start).GetNormalized();
	Vec3 arbitraryVec = (axis.x != 0.f || axis.z != 0.f) ? Vec3(0.f, 1.f, 0.f) : Vec3(1.f, 0.f, 0.f);
	Vec3 Ibasis = CrossProduct3D(arbitraryVec, axis).GetNormalized();
	Vec3 Jbasis = CrossProduct3D(axis, Ibasis).GetNormalized();

	const float theta = 360.0f / static_cast<float>(numSlices);

	for (int i = 0; i < numSlices; i++) 
	{
		Vertex_PCU vertex1, vertex2, vertex3, vertex4;

		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		vertex1.m_position = start + (Ibasis * CosDegrees(thetaCurrent) + Jbasis * SinDegrees(thetaCurrent)) * radius;
		vertex1.m_uvTexCoords = Vec2(uMin, vMin);
		vertex1.m_color = color;

		vertex2.m_position = start + (Ibasis * CosDegrees(thetaNext) + Jbasis * SinDegrees(thetaNext)) * radius;
		vertex2.m_uvTexCoords = Vec2(uMax, vMin);
		vertex2.m_color = color;

		vertex3.m_position = end + (Ibasis * CosDegrees(thetaNext) + Jbasis * SinDegrees(thetaNext)) * radius;
		vertex3.m_uvTexCoords = Vec2(uMax, vMax);
		vertex3.m_color = color;

		vertex4.m_position = end + (Ibasis * CosDegrees(thetaCurrent) + Jbasis * SinDegrees(thetaCurrent)) * radius;
		vertex4.m_uvTexCoords = Vec2(uMin, vMax);
		vertex4.m_color = color;

		verts.emplace_back(vertex1);
		verts.emplace_back(vertex2);
		verts.emplace_back(vertex3);

		verts.emplace_back(vertex3);
		verts.emplace_back(vertex4);
		verts.emplace_back(vertex1);
	}
	
	// Generate the top face
	Vec2 uvCenter = Vec2(UVs.GetCenter().x, UVs.GetCenter().y);
	for (int i = 0; i < numSlices; i++) 
	{
		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		Vertex_PCU vertex1, vertex2, vertexCenter;

		vertex1.m_position = end + (Ibasis * CosDegrees(thetaCurrent) + Jbasis * SinDegrees(thetaCurrent)) * radius;
		vertex1.m_uvTexCoords = Vec2(uvCenter.x + CosDegrees(thetaCurrent) * 0.5f, uvCenter.y + SinDegrees(thetaCurrent) * 0.5f);
		vertex1.m_color = color;

		vertex2.m_position = end + (Ibasis * CosDegrees(thetaNext) + Jbasis * SinDegrees(thetaNext)) * radius;
		vertex2.m_uvTexCoords = Vec2(uvCenter.x + CosDegrees(thetaNext) * 0.5f, uvCenter.y + SinDegrees(thetaNext) * 0.5f);
		vertex2.m_color = color;

		vertexCenter.m_position = end;
		vertexCenter.m_uvTexCoords = uvCenter;
		vertexCenter.m_color = color;

		verts.emplace_back(vertex1);
		verts.emplace_back(vertex2);
		verts.emplace_back(vertexCenter);
	}

	// Generate the bottom face
	for (int i = 0; i < numSlices; i++) 
	{
		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		Vertex_PCU vertex1, vertex2, vertexCenter;

		vertex1.m_position = start + (Ibasis * CosDegrees(thetaCurrent) + Jbasis * SinDegrees(thetaCurrent)) * radius;
		vertex1.m_uvTexCoords = Vec2(uvCenter.x + CosDegrees(thetaCurrent) * 0.5f, uvCenter.y + SinDegrees(thetaCurrent) * 0.5f);
		vertex1.m_color = color;

		vertex2.m_position = start + (Ibasis * CosDegrees(thetaNext) + Jbasis * SinDegrees(thetaNext)) * radius;
		vertex2.m_uvTexCoords = Vec2(uvCenter.x + CosDegrees(thetaNext) * 0.5f, uvCenter.y + SinDegrees(thetaNext) * 0.5f);
		vertex2.m_color = color;

		vertexCenter.m_position = start;
		vertexCenter.m_uvTexCoords = uvCenter;
		vertexCenter.m_color = color;

		verts.emplace_back(vertexCenter);
		verts.emplace_back(vertex2);
		verts.emplace_back(vertex1);
	}

	Mat44 matrix;
	matrix.SetIJKT3D(Ibasis.GetNormalized(), Jbasis.GetNormalized(), axis.GetNormalized(), start);

	for (int index = startIndex; index < verts.size(); index++) 
	{
		verts[index].m_position = matrix.TransformPosition3D(verts[index].m_position);
	}
}

void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices)
{
	int startIndex = static_cast<int>(verts.size());

	Vec3 Kbasis = (end - start).GetNormalized();
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 worldForward = Vec3(1.0f, 0.0f, 0.0f);

	Vec3 JBasis = CrossProduct3D(worldUp, Kbasis).GetNormalized();
	Vec3 IBasis = CrossProduct3D(JBasis, Kbasis);

	if (fabs(DotProduct3D(Kbasis, worldUp)) > 0.99f)
	{
		JBasis = CrossProduct3D(Kbasis, worldForward);
		IBasis = CrossProduct3D(JBasis, Kbasis);
	}

	float length = (end - start).GetLength();

	const float theta = 360.0f / static_cast<float>(numSlices);

	for (int i = 0; i < numSlices; i++)
	{
		Vertex_PCU vertex1, vertex2;

		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		vertex1.m_position = Vec3((CosDegrees(thetaCurrent) * radius), (SinDegrees(thetaCurrent) * radius), 0.f);
		vertex1.m_uvTexCoords = Vec2(uMin, vMin);
		vertex1.m_color = color;

		vertex2.m_position = Vec3((CosDegrees(thetaNext) * radius), (SinDegrees(thetaNext) * radius), 0.f);
		vertex2.m_uvTexCoords = Vec2(uMax, vMax);
		vertex2.m_color = color;

		verts.emplace_back(Vertex_PCU(Vec3(0.f, 0.f, 0.f), color, Vec2(UVs.m_mins.x, UVs.m_maxs.x)));
		verts.emplace_back(vertex1);
		verts.emplace_back(vertex2);

		verts.emplace_back(Vertex_PCU(Vec3(0.f, 0.f, length), color, Vec2(UVs.m_mins.x, UVs.m_maxs.x)));
		verts.emplace_back(vertex2);
		verts.emplace_back(vertex1);
	}

	Mat44 matrix;
	matrix.SetIJKT3D(IBasis.GetNormalized(), JBasis.GetNormalized(), Kbasis.GetNormalized(), start);
	
	for (int index = startIndex; index < verts.size(); index++)
	{
		verts[index].m_position = matrix.TransformPosition3D(verts[index].m_position);
	}
}

void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& start, const Vec3& end, float coneHeight, float radius, int numSlices, const Rgba8& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int startIndex = static_cast<int>(verts.size());

	// Basis vectors
	Vec3 Kbasis = (end - start).GetNormalized();
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 worldForward = Vec3(1.0f, 0.0f, 0.0f);

	Vec3 JBasis = CrossProduct3D(worldUp, Kbasis).GetNormalized();
	Vec3 IBasis = CrossProduct3D(JBasis, Kbasis);

	if (fabs(DotProduct3D(Kbasis, worldUp)) > 0.99f)
	{
		JBasis = CrossProduct3D(Kbasis, worldForward);
		IBasis = CrossProduct3D(JBasis, Kbasis);
	}

	const float theta = 360.0f / static_cast<float>(numSlices);

	// Top vertex (tip of the cone)
	int tipIndex = static_cast<int>(verts.size());
	verts.emplace_back(Vertex_PCU(Vec3(0.f, 0.f, coneHeight), color, Vec2(0.5f, 1.0f)));

	// Base center vertex
	int baseCenterIndex = static_cast<int>(verts.size());
	verts.emplace_back(Vertex_PCU(Vec3(0.f, 0.f, 0.f), color, Vec2(0.5f, 0.f)));

	// Generate base ring vertices
	std::vector<int> baseIndices;
	for (int i = 0; i < numSlices; i++)
	{
		float thetaCurrent = theta * i;
		float u = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float v = UVs.m_mins.y;

		Vec3 basePosition = Vec3(CosDegrees(thetaCurrent) * radius, SinDegrees(thetaCurrent) * radius, 0.0f);
		baseIndices.emplace_back(static_cast<int>(verts.size()));
		verts.emplace_back(Vertex_PCU(basePosition, color, Vec2(u, v)));
	}

	// Create indices for the cone's sides
	for (int i = 0; i < numSlices; i++)
	{
		int nextIndex = (i + 1) % numSlices;
		indexes.emplace_back(tipIndex);
		indexes.emplace_back(baseIndices[i]);
		indexes.emplace_back(baseIndices[nextIndex]);
	}

	// Create indices for the base
	for (int i = 0; i < numSlices; i++)
	{
		int nextIndex = (i + 1) % numSlices;
		indexes.emplace_back(baseCenterIndex);
		indexes.emplace_back(baseIndices[nextIndex]);
		indexes.emplace_back(baseIndices[i]);
	}

	// Transform all vertices into world space
	Mat44 matrix;
	matrix.SetIJKT3D(IBasis.GetNormalized(), JBasis.GetNormalized(), Kbasis.GetNormalized(), start);

	for (int i = startIndex; i < verts.size(); i++)
	{
		verts[i].m_position = matrix.TransformPosition3D(verts[i].m_position);
	}
}

void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float interpolate, float cylinderRadius, float coneRadius, const Rgba8& coneColor, const Rgba8& cylinderColor, const AABB2& UVs, int numCylinderSlices, int numconeSlices)
{
	Vec3 cylinderEnd = start + interpolate * (end - start);

	AddVertsForZCylinder3D(verts, start, cylinderEnd, cylinderRadius, numCylinderSlices, cylinderColor, UVs);
	AddVertsForCone3D(verts, cylinderEnd, end, coneRadius, coneColor, UVs, numconeSlices);
}

void AddVertsForZCapsule3D(std::vector<Vertex_PCU>& verts, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numSlices)
{
	Vec3 start = capsule.m_start;
	Vec3 end = capsule.m_end;
	float radius = capsule.m_radius;
	const float theta = 360.0f / static_cast<float>(numSlices);

	// Generate vertices for the cylindrical part
	for (int i = 0; i < numSlices; i++) 
	{
		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		Vec3 vertex1Bottom = Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), 0.f) + start;
		Vec3 vertex2Bottom = Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), 0.f) + start;
		Vec3 vertex1Top = Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), 0.f) + end;
		Vec3 vertex2Top = Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), 0.f) + end;

		AABB2 correctedUVs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));
		AddVertsForQuad3D(verts, vertex1Bottom, vertex2Bottom, vertex2Top, vertex1Top, color, correctedUVs);
	}

	// Generate vertices for the top (north) hemisphere
	AddVertsForNorthZHemisphere(verts, end, radius, color, UVs, numSlices);

	// Generate vertices for the bottom (south) hemisphere
	AddVertsForSouthZHemisphere(verts, start, radius, color, UVs, numSlices);
}

void AddVertsForZCapsule3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numLatitudeSlices, int numLongitudeSlices)
{
	Vec3 start = capsule.m_start;
	Vec3 end = capsule.m_end;
	float radius = capsule.m_radius;
	const float thetaLongitude = 360.f / static_cast<float>(numLongitudeSlices);

	// Generate vertices for the cylindrical part
	for (int i = 0; i < numLongitudeSlices; i++)
	{
		float thetaCurrent = thetaLongitude * i;
		float thetaNext = thetaLongitude * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		Vec3 vertex1Bottom = Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), 0.f) + start;
		Vec3 vertex2Bottom = Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), 0.f) + start;
		Vec3 vertex1Top = Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), 0.f) + end;
		Vec3 vertex2Top = Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), 0.f) + end;

		AABB2 correctedUVs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

		verts.emplace_back(Vertex_PCU(vertex1Bottom, color, Vec2(uMin, vMin)));
		verts.emplace_back(Vertex_PCU(vertex2Bottom, color, Vec2(uMax, vMin)));
		verts.emplace_back(Vertex_PCU(vertex1Top, color, Vec2(uMin, vMax)));

		verts.emplace_back(Vertex_PCU(vertex2Bottom, color, Vec2(uMax, vMin)));
		verts.emplace_back(Vertex_PCU(vertex2Top, color, Vec2(uMax, vMax)));
		verts.emplace_back(Vertex_PCU(vertex1Top, color, Vec2(uMin, vMax)));

		int indexBase = static_cast<int>(verts.size()) - 6;
		indexes.emplace_back(indexBase);     // First triangle
		indexes.emplace_back(indexBase + 1);
		indexes.emplace_back(indexBase + 2);

		indexes.emplace_back(indexBase + 3); // Second triangle
		indexes.emplace_back(indexBase + 4);
		indexes.emplace_back(indexBase + 5);
	}

	// Generate vertices for the top (north) hemisphere
	AddVertsForNorthZHemisphere(verts, indexes, end, radius, color, UVs, numLatitudeSlices, numLongitudeSlices);

	// Generate vertices for the bottom (south) hemisphere
	AddVertsForSouthZHemisphere(verts, indexes, start, radius, color, UVs, numLatitudeSlices, numLongitudeSlices);
}

void AddVertsForCapsule3D(std::vector<Vertex_PCU>& verts, const Mat44& transform, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numSlices)
{
	Vec3 start = capsule.m_start;
	Vec3 end = capsule.m_end;
	float radius = capsule.m_radius;

	float length = (end.z - start.z - 2.f * radius);
	const float theta = 360.0f / static_cast<float>(numSlices);

	// Generate vertices for the cylindrical part
	for (int i = 0; i < numSlices; i++) 
	{
		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		Vec3 vertex1Bottom = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), 0.f) + Vec3(start.x, start.y, start.z + radius));
		Vec3 vertex2Bottom = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), 0.f) + Vec3(start.x, start.y, start.z + radius));
		Vec3 vertex1Top = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), length) + Vec3(start.x, start.y, start.z + radius));
		Vec3 vertex2Top = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), length) + Vec3(start.x, start.y, start.z + radius));

		AABB2 correctedUVs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));
		AddVertsForQuad3D(verts, vertex1Bottom, vertex2Bottom, vertex2Top, vertex1Top, color, correctedUVs);
	}

	// Generate vertices for the top (north) hemisphere
	AddVertsForHemisphere(verts, transform, Vec3(end.x, end.y, end.z - radius), radius, color, UVs, numSlices, true);

	// Generate vertices for the bottom (south) hemisphere
	AddVertsForHemisphere(verts, transform, Vec3(start.x, start.y, start.z + radius), radius, color, UVs, numSlices, false);
}

void AddVertsForCapsule3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Mat44& transform, Capsule3 const& capsule, Rgba8 const& color, const AABB2& UVs, int numSlices)
{
	Vec3 start = capsule.m_start;
	Vec3 end = capsule.m_end;
	float radius = capsule.m_radius;

	float length = (end.z - start.z - 2.f * radius);
	const float theta = 360.f / static_cast<float>(numSlices);

	// Generate vertices for the cylindrical part
	for (int i = 0; i < numSlices; i++) 
	{
		float thetaCurrent = theta * i;
		float thetaNext = theta * (i + 1);

		float uMin = RangeMapClamped(thetaCurrent, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float uMax = RangeMapClamped(thetaNext, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float vMin = UVs.m_mins.y;
		float vMax = UVs.m_maxs.y;

		Vec3 vertex1Bottom = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), 0.f) + Vec3(start.x, start.y, start.z + radius));
		Vec3 vertex2Bottom = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), 0.f) + Vec3(start.x, start.y, start.z + radius));
		Vec3 vertex1Top = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaCurrent), radius * SinDegrees(thetaCurrent), length) + Vec3(start.x, start.y, start.z + radius));
		Vec3 vertex2Top = transform.TransformPosition3D(Vec3(radius * CosDegrees(thetaNext), radius * SinDegrees(thetaNext), length) + Vec3(start.x, start.y, start.z + radius));

		AABB2 correctedUVs = AABB2(Vec2(uMin, vMin), Vec2(uMax, vMax));

		verts.emplace_back(Vertex_PCU(vertex1Bottom, color, Vec2(uMin, vMin)));
		verts.emplace_back(Vertex_PCU(vertex2Bottom, color, Vec2(uMax, vMin)));
		verts.emplace_back(Vertex_PCU(vertex1Top, color, Vec2(uMin, vMax)));

		verts.emplace_back(Vertex_PCU(vertex2Bottom, color, Vec2(uMax, vMin)));
		verts.emplace_back(Vertex_PCU(vertex2Top, color, Vec2(uMax, vMax)));
		verts.emplace_back(Vertex_PCU(vertex1Top, color, Vec2(uMin, vMax)));

		int indexBase = static_cast<int>(verts.size()) - 6;
		indexes.emplace_back(indexBase);     // First triangle
		indexes.emplace_back(indexBase + 1);
		indexes.emplace_back(indexBase + 2);

		indexes.emplace_back(indexBase + 3); // Second triangle
		indexes.emplace_back(indexBase + 4);
		indexes.emplace_back(indexBase + 5);
	}

	// Generate vertices for the bottom (south) hemisphere
	AddVertsForHemisphere(verts, indexes, transform, Vec3(start.x, start.y, start.z + radius), radius, color, UVs, numSlices, false);

	// Generate vertices for the top (north) hemisphere
	AddVertsForHemisphere(verts, indexes, transform, Vec3(end.x, end.y, end.z - radius), radius, color, UVs, numSlices, true);
}

void AddVertsForPlane3D(std::vector<Vertex_PCU>& verts, const Plane3D& plane, int halfIterations)
{
	Vec3 closestPointToOrigin = plane.m_normal * plane.m_distance;

	AddVertsForZSphere(verts, closestPointToOrigin, 0.05f, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 32);
	AddVertsForArrow3D(verts, closestPointToOrigin, closestPointToOrigin + plane.m_normal, .7f, .05f, .075f, Rgba8::LIGHT_GRAY, Rgba8::LIGHT_GRAY, AABB2::ZERO_TO_ONE, 64, 64);

	Vec3 worldUp = Vec3(0.f, 0.f, 1.f);
	Vec3 tangent = CrossProduct3D(worldUp, plane.m_normal).GetNormalized();
	Vec3 bitangent = CrossProduct3D(tangent, plane.m_normal).GetNormalized();

	int colorScale = 255 / halfIterations;

	for (int i = -halfIterations; i < halfIterations + 1; i++)
	{
		int absI = halfIterations - abs(i);

		Vec3 x = closestPointToOrigin + (bitangent * (float)i);
		Vec3 y = closestPointToOrigin + (tangent * (float)i);

		Rgba8 xAxis = Rgba8(255, 0, 0, (unsigned char)absI * (unsigned char)colorScale);
		Rgba8 yAxis = Rgba8(0, 255, 0, (unsigned char)absI * (unsigned char)colorScale);

		AddVertsForArrow3D(verts, x, closestPointToOrigin + (tangent * (float)halfIterations) + (bitangent * (float)i), .7f, .05f, .075f, xAxis, xAxis, AABB2::ZERO_TO_ONE, 32, 64);
		AddVertsForArrow3D(verts, x, closestPointToOrigin - (tangent * (float)halfIterations) + (bitangent * (float)i), .7f, .05f, .075f, xAxis, xAxis, AABB2::ZERO_TO_ONE, 32, 64);
																																	  
		AddVertsForArrow3D(verts, y, closestPointToOrigin + (bitangent * (float)halfIterations) + (tangent * (float)i), .7f, .05f, .075f, yAxis, yAxis, AABB2::ZERO_TO_ONE, 32, 64);
		AddVertsForArrow3D(verts, y, closestPointToOrigin - (bitangent * (float)halfIterations) + (tangent * (float)i), .7f, .05f, .075f, yAxis, yAxis, AABB2::ZERO_TO_ONE, 32, 64);
	}
}

void AddVertsForPlane3D(std::vector<Vertex_PCU>& verts, const Plane3D& plane, float distance, const Rgba8& color, const AABB2& UVs)
{
	Vec3 closestPointToOrigin = plane.m_normal * plane.m_distance;

	Vec3 worldUp = Vec3(0.f, 0.f, 1.f);
	if (fabs(DotProduct3D(plane.m_normal, worldUp)) > 0.99f) 
	{
		// The normal is close to being parallel to worldUp, so use an alternate vector for cross products
		worldUp = Vec3(1.f, 0.f, 0.f); // Arbitrary vector for nearly vertical planes
	}

	Vec3 tangent = CrossProduct3D(worldUp, plane.m_normal).GetNormalized();
	Vec3 bitangent = CrossProduct3D(tangent, plane.m_normal).GetNormalized();

	// Calculate the corners of the quad
	Vec3 bottomLeft = closestPointToOrigin - (tangent * distance * 0.5f) - (bitangent * distance * 0.5f);
	Vec3 bottomRight = closestPointToOrigin + (tangent * distance * 0.5f) - (bitangent * distance * 0.5f);
	Vec3 topRight = closestPointToOrigin + (tangent * distance * 0.5f) + (bitangent * distance * 0.5f);
	Vec3 topLeft = closestPointToOrigin - (tangent * distance * 0.5f) + (bitangent * distance * 0.5f);

	AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, color, UVs);
}

void AddVertsForPrism(std::vector<Vertex_PCU>& verts, const Vec3& baseCenter, float baseLength, float baseWidth, float prismHeight, Rgba8 const& color, const AABB2& UVs)
{
	// Bottom face
	Vec3 bottomLeft(baseCenter.x - baseLength / 2, baseCenter.y - baseWidth / 2, baseCenter.z);  // Bottom-left corner
	Vec3 bottomRight(baseCenter.x + baseLength / 2, baseCenter.y - baseWidth / 2, baseCenter.z); // Bottom-right corner
	Vec3 topLeft(baseCenter.x - baseLength / 2, baseCenter.y + baseWidth / 2, baseCenter.z);	 // Top-left corner
	Vec3 topRight(baseCenter.x + baseLength / 2, baseCenter.y + baseWidth / 2, baseCenter.z);    // Top-right corner
	AddVertsForQuad3D(verts, topLeft, topRight, bottomRight, bottomLeft, color, UVs);            // Bottom face

	// Side faces
	Vec3 apexLeft(baseCenter.x - baseLength / 2, baseCenter.y, baseCenter.z + prismHeight);
	Vec3 apexRight(baseCenter.x + baseLength / 2, baseCenter.y, baseCenter.z + prismHeight);
	AddVertsForQuad3D(verts, bottomLeft, bottomRight, apexRight, apexLeft, color, UVs); // First slanted face
	AddVertsForQuad3D(verts, apexLeft, apexRight, topRight, topLeft, color, UVs);       // Second slanted face

	AddVertsFor3DTriangle(verts, topLeft, bottomLeft, apexLeft, color, UVs);      // First triangle
	AddVertsFor3DTriangle(verts, bottomRight, topRight, apexRight, color, UVs); // Second triangle
}

void AddVertsForPrism(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& baseCenter, float baseLength, float baseWidth, float prismHeight, Rgba8 const& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	// Bottom face
	Vec3 bottomLeft(baseCenter.x - baseLength / 2, baseCenter.y - baseWidth / 2, baseCenter.z);  // Bottom-left corner
	Vec3 bottomRight(baseCenter.x + baseLength / 2, baseCenter.y - baseWidth / 2, baseCenter.z); // Bottom-right corner
	Vec3 topLeft(baseCenter.x - baseLength / 2, baseCenter.y + baseWidth / 2, baseCenter.z);	 // Top-left corner
	Vec3 topRight(baseCenter.x + baseLength / 2, baseCenter.y + baseWidth / 2, baseCenter.z);    // Top-right corner
	AddVertsForQuad3D(verts, indexes, topLeft, topRight, bottomRight, bottomLeft, color, UVs);            // Bottom face

	// Side faces
	Vec3 apexLeft(baseCenter.x - baseLength / 2, baseCenter.y, baseCenter.z + prismHeight);
	Vec3 apexRight(baseCenter.x + baseLength / 2, baseCenter.y, baseCenter.z + prismHeight);
	AddVertsForQuad3D(verts, indexes, bottomLeft, bottomRight, apexRight, apexLeft, color, UVs); // First slanted face
	AddVertsForQuad3D(verts, indexes, apexLeft, apexRight, topRight, topLeft, color, UVs);       // Second slanted face

	AddVertsFor3DTriangle(verts, indexes, topLeft, bottomLeft, apexLeft, color, UVs);      // First triangle
	AddVertsFor3DTriangle(verts, indexes, topRight, bottomRight, apexRight, color, UVs); // Second triangle
}

void AddVertsForPyramid(std::vector<Vertex_PCU>& verts, const Vec3& baseCenter, float baseLength, float baseHeight, float pyramidHeight, Rgba8 const& color, const AABB2& UVs)
{
	Vec3 bottomLeft(baseCenter.x - baseLength / 2, baseCenter.y - baseHeight / 2, baseCenter.z);
	Vec3 bottomRight(baseCenter.x + baseLength / 2, baseCenter.y - baseHeight / 2, baseCenter.z);
	Vec3 topLeft(baseCenter.x - baseLength / 2, baseCenter.y + baseHeight / 2, baseCenter.z);
	Vec3 topRight(baseCenter.x + baseLength / 2, baseCenter.y + baseHeight / 2, baseCenter.z);

	Vec3 apex(baseCenter.x, baseCenter.y, baseCenter.z + pyramidHeight);

	AddVertsForQuad3D(verts, topLeft, topRight, bottomRight, bottomLeft, color, UVs);  // Base face

	// Side faces (triangles)
	AddVertsFor3DTriangle(verts, bottomLeft, bottomRight, apex, color);
	AddVertsFor3DTriangle(verts, bottomRight, topRight, apex, color);
	AddVertsFor3DTriangle(verts, topRight, topLeft, apex, color);
	AddVertsFor3DTriangle(verts, topLeft, bottomLeft, apex, color);
}

void AddVertsForPyramid(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& baseCenter, float baseLength, float baseHeight, float pyramidHeight, Rgba8 const& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 bottomLeft(baseCenter.x - baseLength / 2, baseCenter.y - baseHeight / 2, baseCenter.z);
	Vec3 bottomRight(baseCenter.x + baseLength / 2, baseCenter.y - baseHeight / 2, baseCenter.z);
	Vec3 topLeft(baseCenter.x - baseLength / 2, baseCenter.y + baseHeight / 2, baseCenter.z);
	Vec3 topRight(baseCenter.x + baseLength / 2, baseCenter.y + baseHeight / 2, baseCenter.z);

	Vec3 apex(baseCenter.x, baseCenter.y, baseCenter.z + pyramidHeight);

	AddVertsForQuad3D(verts, indexes, topLeft, topRight, bottomRight, bottomLeft, color, UVs);  // Base face

	// Side faces (triangles)
	AddVertsFor3DTriangle(verts, indexes, bottomLeft, bottomRight, apex, color);
	AddVertsFor3DTriangle(verts, indexes, bottomRight, topRight, apex, color);
	AddVertsFor3DTriangle(verts, indexes, topRight, topLeft, apex, color);
	AddVertsFor3DTriangle(verts, indexes, topLeft, bottomLeft, apex, color);
}
