#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <sstream>

BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture)
	: m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension), m_fontGlyphsSpriteSheet(SpriteSheet(&fontTexture, IntVec2(16, 16)))
{
	if (m_fontTier == Fontier::TIER2_PROPORTIONAL_GRID)
	{
		InitializeTier2GlyphWidths();
	}
}

Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::InitializeTier2GlyphWidths()
{
	for (int i = 32; i < 127; ++i)
	{
		char c = static_cast<char>(i);
		switch (c)
		{
		case 'i': case 'l': case '!': case '.':
			m_glyphWidths[i] = 0.25f;
			break;
		case 'W': case 'M':
			m_glyphWidths[i] = 1.2f;
			break;
		case ' ':
			m_glyphWidths[i] = 0.25f;
			break;
		default:
			m_glyphWidths[i] = 1.f;
			break;
		}
	}
}

void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextDrawMode mode, int maxGlyphsToDraw)
{
	UNUSED(maxGlyphsToDraw);

	Strings splitString = SplitStringOnDelimiter(text, '\n');
	int numTextLines = static_cast<int>(splitString.size());
	
	float textHeight = static_cast<float>(numTextLines) * cellHeight;
	int longestLineWidth = 0;
	int longestLineIndex = 0;

	for (int lineIndex = 0; lineIndex < numTextLines; lineIndex++)
	{
		if (splitString[lineIndex].length() > longestLineWidth)
		{
			longestLineIndex = lineIndex;
			longestLineWidth = static_cast<int>(splitString[lineIndex].length()); // num characters wide 
		}
	}

	float cellWidth = cellHeight * cellAspect;
	float textWidth = cellWidth * static_cast<float>(longestLineWidth);

	Vec2 boxDimensions = box.GetDimensions();
	// Get width of box in world units 
	float boxWidth = boxDimensions.x;
	// Get height of box in world units 
	float boxHeight = boxDimensions.y;

	// Set Bounds of box for text
	AABB2 setBox(box.m_mins, Vec2(box.m_mins.x + boxWidth, box.m_mins.y + boxHeight));

	//Set the position to start at the mins
	Vec2 textPosition = setBox.m_mins;

	textPosition.y += alignment.y * (boxHeight - cellHeight);

	switch (mode)
	{
	case TextDrawMode::SHRINK_TO_FIT:
		if (textWidth > setBox.GetDimensions().x)
		{
			// Handle shrink to fit(next line exceeds box height)
			// Calculate the scaling factor to fit the entire text within the box
			float xScalingFactor = (setBox.m_maxs.x - setBox.m_mins.x) / textWidth;
			float yScalingFactor = (setBox.m_maxs.y - setBox.m_mins.y) / textHeight;

			// Apply scaling factor to cell height and width
			if (xScalingFactor < 1.f)
			{
				cellHeight *= xScalingFactor;
			}

			if (yScalingFactor < 1.f)
			{
				cellHeight *= yScalingFactor;
			}
		}
		[[fallthrough]]; // tells the compiler that skipping break is intentional, preventing warnings.
	case TextDrawMode::OVERRUN:
		for (int lineIndex = 0; lineIndex < numTextLines; lineIndex++)
		{
			textPosition.x = setBox.m_mins.x;

			// align the text to the box based the alignment 
			float lineWidth = GetTextWidth(cellHeight, splitString[lineIndex], cellAspect); //* static_cast<float>(maxGlyphsToDraw);
			textPosition.x += alignment.x * (boxWidth - lineWidth);

			if (lineIndex != 0)
			{
				textPosition.y -= cellHeight;
			}

			// Draw the text
			AddVertsForText2D(vertexArray, textPosition, cellHeight, splitString[lineIndex], tint, cellAspect);
		}
		[[fallthrough]]; // tells the compiler that skipping break is intentional, preventing warnings.
	case TextDrawMode::WRAP:
		std::vector<std::string> wrappedLines;
		std::string currentLine;
		float currentLineWidth = 0.f;
		float maxLineWidth = setBox.GetDimensions().x;

		for (char c : text)
		{
			float charWidth = GetTextWidth(cellHeight, std::string(1, c), cellAspect);

			if (currentLineWidth + charWidth > maxLineWidth && !currentLine.empty())
			{
				wrappedLines.emplace_back(currentLine);
				currentLine = "";
				currentLineWidth = 0.f;
			}

			currentLine += c;
			currentLineWidth += charWidth;
		}

		if (!currentLine.empty())
		{
			wrappedLines.emplace_back(currentLine);
		}

		for (int lineIndex = 0; lineIndex < static_cast<int>(wrappedLines.size()); lineIndex++)
		{
			textPosition.x = setBox.m_mins.x;
			float lineWidth = GetTextWidth(cellHeight, wrappedLines[lineIndex], cellAspect);
			textPosition.x += alignment.x * (boxWidth - lineWidth);

			if (lineIndex != 0)
			{
				textPosition.y -= cellHeight;
			}

			AddVertsForText2D(vertexArray, textPosition, cellHeight, wrappedLines[lineIndex], tint, cellAspect);
		}
	}
}

void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	switch (m_fontTier)
	{
	case Fontier::TIER1_FIXED_GRID:
		AddVerts_Tier1(vertexArray, textMins, cellHeight, text, tint, cellAspect);
		break;
	case Fontier::TIER2_PROPORTIONAL_GRID:
		AddVerts_Tier2(vertexArray, textMins, cellHeight, text, tint, cellAspect);
		break;
	case Fontier::TIER3_BMFONT:
		AddVerts_Tier3(vertexArray, textMins, cellHeight, text, tint, cellAspect);
		break;
	}
}

void BitmapFont::SetFontTier(Fontier tier)
{
	m_fontTier = tier;
}

int BitmapFont::GetWrappedTextCount(std::string const& text, float maxWidth, float cellHeight, float cellAspect)
{
	int numLines = 1;
	float currentWidth = 0.f;

	for (char c : text)
	{
		float charWidth = GetTextWidth(cellHeight, std::string(1, c), cellAspect);

		if (currentWidth + charWidth > maxWidth)
		{
			numLines++;
			currentWidth = 0.f;
		}

		currentWidth += charWidth;
	}

	return numLines;
}

void BitmapFont::AddVertsForText3DAtOriginXForward(std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, int maxGlyphsToDraw /*= 999999999*/)
{
	UNUSED(maxGlyphsToDraw);

	Strings splitString = SplitStringOnDelimiter(text, '\n');
	int numTextLines = static_cast<int>(splitString.size());

	for (int lineIndex = 0; lineIndex < numTextLines; lineIndex++)
	{
		Vec2 textMins;
		textMins.x = 0.0f; // Start at the origin for each line
		textMins.y = cellHeight * (numTextLines - lineIndex - 1); // Adjust y-position for each line
		AddVertsForText2D(verts, textMins, cellHeight, splitString[lineIndex], tint, cellAspect);
	}

	// Use the alignment to translated/determine which axis the text will display on 
	AABB2 vertexBounds = GetVertexBounds2D(verts);
	Vec3 translation = Vec3(((vertexBounds.m_mins.x - vertexBounds.m_maxs.x) * alignment.x), ((vertexBounds.m_mins.y - vertexBounds.m_maxs.y) * alignment.y), 0.f);
	Mat44 matrix;
	matrix.AppendYRotation(90.f);
	matrix.AppendZRotation(90.f);
	matrix.AppendTranslation3D(translation);
	TransformVertexArray3D(verts, matrix);
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect)
{
	if (m_fontTier == Fontier::TIER1_FIXED_GRID)
	{
		return static_cast<float>(text.length()) * cellHeight * cellAspect;
	}
	else if (m_fontTier == Fontier::TIER2_PROPORTIONAL_GRID)
	{
		float totalWidth = 0.f;

		for (char c : text)
		{
			float widthScale = 1.f;
			auto it = m_glyphWidths.find(static_cast<int>(c));
			if (it != m_glyphWidths.end())
			{
				widthScale = it->second;
			}

			totalWidth += cellHeight * cellAspect * widthScale;
		}

		return totalWidth;
	}

	return 0.f;
}

void BitmapFont::AddVerts_Tier1(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	float cellWidth = cellHeight * cellAspect;

	for (size_t characterIndex = 0; characterIndex < text.size(); characterIndex++)
	{
		float minX = textMins.x + cellWidth * characterIndex;
		float minY = textMins.y;
		float maxX = minX + cellWidth;
		float maxY = textMins.y + cellHeight;

		AABB2 characterBounds(Vec2(minX, minY), Vec2(maxX, maxY));

		AABB2 Uvs = m_fontGlyphsSpriteSheet.GetSpriteUVs(text[characterIndex]);
		AddVertsForAABB2D(vertexArray, characterBounds, tint, Uvs.m_mins, Uvs.m_maxs);
	}
}

void BitmapFont::AddVerts_Tier2(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	Vec2 penPosition = textMins;

	for (size_t i = 0; i < text.size(); i++)
	{
		int glyphID = static_cast<int>(text[i]);

		// Fallback if no width info
		float widthScale = 1.0f;
		auto it = m_glyphWidths.find(glyphID);
		if (it != m_glyphWidths.end())
		{
			widthScale = it->second;
		}

		float glyphWidth = cellHeight * cellAspect * widthScale;
		AABB2 glyphBounds(penPosition, penPosition + Vec2(glyphWidth, cellHeight));

		AABB2 uvs = m_fontGlyphsSpriteSheet.GetSpriteUVs(glyphID);
		AddVertsForAABB2D(vertexArray, glyphBounds, tint, uvs.m_mins, uvs.m_maxs);

		penPosition.x += glyphWidth; // Move pen to the right
	}
}

void BitmapFont::AddVerts_Tier3(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	UNUSED(vertexArray);
	UNUSED(textMins);
	UNUSED(cellHeight);
	UNUSED(text);
	UNUSED(tint);
	UNUSED(cellAspect);
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	return static_cast<float>(glyphUnicode);
}
