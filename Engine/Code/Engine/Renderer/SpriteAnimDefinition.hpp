#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/StringUtils.hpp"

enum class SpriteAnimPlaybackType
{
	NONE = -1,
	ONCE, // For 5-frame animation. Plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP, // For 5-frame animation. Plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG, // For 5-frame animation. Plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};

class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition();
	~SpriteAnimDefinition();

	SpriteAnimDefinition(SpriteSheet* sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType);

	const SpriteDefinition& GetSpriteDefAtTime(float seconds, float playbackSpeed = 1.f) const;

	float GetDuration() const;

private:
	SpriteSheet* m_spriteSheet = nullptr;
	int m_startSpriteIndex = -1;
	int m_endSpriteIndex = -1;
	float m_durationSeconds = 1.f;
	SpriteAnimPlaybackType m_playbackType = SpriteAnimPlaybackType::LOOP;
};