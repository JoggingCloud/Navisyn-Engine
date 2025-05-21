#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class Texture;
class SpriteDefinition;

class SpriteSheet
{
public:
	SpriteSheet();
	~SpriteSheet();
	explicit SpriteSheet(Texture* texture, IntVec2 const& simpleGridLayout);

	Texture& GetTexture() const;
	int GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef(int spriteIndex) const;
	void GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const;
	AABB2 GetSpriteUVs(int spriteIndex) const;
	AABB2 GetSpriteUVs(const IntVec2& spriteCoords) const;

protected:
	Texture* m_texture;
	int m_spriteSheetWidth = 0;
	std::vector<SpriteDefinition> m_spriteDefs;
};