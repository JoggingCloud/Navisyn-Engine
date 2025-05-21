#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteAnimDefinition::~SpriteAnimDefinition()
{

}

SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet* sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType)
	: m_spriteSheet(sheet), m_startSpriteIndex(startSpriteIndex), m_endSpriteIndex(endSpriteIndex), m_durationSeconds(durationSeconds), m_playbackType(playbackType)
{

}

SpriteAnimDefinition::SpriteAnimDefinition()
{
	m_spriteSheet = new SpriteSheet();
	m_startSpriteIndex = 0;
	m_endSpriteIndex = 0;
	m_durationSeconds = 0.f;
	m_playbackType = SpriteAnimPlaybackType::ONCE;
}

const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds, float playbackSpeed) const
{
	int animLength = (m_endSpriteIndex - m_startSpriteIndex) + 1;
	float spritePerSecond = static_cast<float>(animLength) / m_durationSeconds;
	int spriteIndex = RoundDownToInt(seconds * spritePerSecond) + m_startSpriteIndex;

	switch (m_playbackType)
	{
	case SpriteAnimPlaybackType::ONCE:
		if (seconds > m_durationSeconds)
		{
			return m_spriteSheet->GetSpriteDef(m_endSpriteIndex);
		}

		if (seconds <= 0.f)
		{
			return m_spriteSheet->GetSpriteDef(m_startSpriteIndex);
		}
			break;
	case SpriteAnimPlaybackType::LOOP:
		spriteIndex = (RoundDownToInt(seconds + playbackSpeed) % (RoundDownToInt(spritePerSecond + playbackSpeed))) % (animLength) + m_startSpriteIndex;
				break;
	case SpriteAnimPlaybackType::PINGPONG:
		int totalFrames = RoundDownToInt(m_durationSeconds * spritePerSecond);
		int pingPongIndex = RoundDownToInt(seconds * spritePerSecond) % (2 * totalFrames);

		if (pingPongIndex >= totalFrames)
		{
			// Play backwards by doubling the frames and ensuring it goes in the negative direction
			pingPongIndex = (2 * totalFrames) - 1 - pingPongIndex;
		}

		// Ensure sprite index stays within the valid range of the animation frames
		spriteIndex = pingPongIndex % animLength;

		break;
	}

	return m_spriteSheet->GetSpriteDef(abs(spriteIndex));
}

float SpriteAnimDefinition::GetDuration() const
{
	return m_durationSeconds;
}
