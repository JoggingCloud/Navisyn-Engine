#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include <map>

enum class Fontier
{
	TIER1_FIXED_GRID,
	TIER2_PROPORTIONAL_GRID,
	TIER3_BMFONT
};

enum class TextDrawMode
{
	SHRINK_TO_FIT,
	OVERRUN,
	WRAP,
	NUM_TEXT_MODES,
};

class BitmapFont
{
	friend class Renderer; // Only the Renderer can create new BitmapFont objects!

private:
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture);

public:
	BitmapFont() = default;
	~BitmapFont() = default;
	Texture& GetTexture();

	void InitializeTier2GlyphWidths();

	void AddVertsForText3DAtOriginXForward(std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, int maxGlyphsToDraw = 999999999);

	void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight,
		std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f,
		Vec2 const& alignment = Vec2(.5f, .5f), TextDrawMode mode = TextDrawMode::SHRINK_TO_FIT, int maxGlyphsToDraw = 99999999);

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect);

	void SetFontTier(Fontier tier);

	int GetWrappedTextCount(std::string const& text, float maxWidth, float cellHeight, float cellAspect);
	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect);

public:
	void AddVerts_Tier1(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect);
	void AddVerts_Tier2(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect);
	void AddVerts_Tier3(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect);

	Fontier m_fontTier = Fontier::TIER1_FIXED_GRID;

protected:
	float GetGlyphAspect(int glyphUnicode) const; // For now this will always return 1.0f!!!

protected:
	std::map<int, float> m_glyphWidths; // int = character code, float = normalized width (0–1)
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet	m_fontGlyphsSpriteSheet;
};
