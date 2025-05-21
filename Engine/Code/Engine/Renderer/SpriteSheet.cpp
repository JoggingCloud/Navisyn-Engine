#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"

SpriteSheet::SpriteSheet(Texture* texture, IntVec2 const& simpleGridLayout)
	: m_texture(texture)
{
	m_spriteSheetWidth = simpleGridLayout.x;

	// Calculate the width and height of each sprite based on the grid layout
	float spriteWidth = 1.0f / simpleGridLayout.x;
	float spriteHeight = 1.0f / simpleGridLayout.y;

	float uNudge = 1.0f / 262144.0f;
	float vNudge = 1.0f / 65536.0f;

	for (int y = 0; y < simpleGridLayout.y; ++y) 
	{
		for (int x = 0; x < simpleGridLayout.x; ++x) 
		{
			Vec2 uvAtMins((x * spriteWidth) + uNudge, 1.0f - (y * spriteHeight) - spriteHeight + vNudge);
			Vec2 uvAtMaxs(uvAtMins.x + spriteWidth - uNudge, 1.0f - (y * spriteHeight) - vNudge);

			// Create a SpriteDefinition for each sprite and add it to m_spriteDefs vector
			m_spriteDefs.emplace_back(*this, y * simpleGridLayout.x + x, uvAtMins, uvAtMaxs);
		}
	}
}

SpriteSheet::SpriteSheet()
{
	m_texture = new Texture();
}

SpriteSheet::~SpriteSheet()
{

}

Texture& SpriteSheet::GetTexture() const
{
	return *m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return static_cast<int>(m_spriteDefs.size());
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMins, out_uvAtMaxs);
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	if (spriteIndex < 0 || spriteIndex >= m_spriteDefs.size())
	{
		return AABB2::ZERO_TO_ONE;
	}
	return m_spriteDefs[spriteIndex].GetUVs();
}

AABB2 SpriteSheet::GetSpriteUVs(const IntVec2& spriteCoords) const
{
	int spriteIndex = spriteCoords.y * m_spriteSheetWidth + spriteCoords.x;
	return m_spriteDefs[spriteIndex].GetUVs();
}

