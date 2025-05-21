#include "Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <chrono>
#include <algorithm>

static Clock s_systemClock;

Clock::Clock(Clock& parent)
	:m_parent(&parent)
{
	parent.AddChild(this);
}

Clock::Clock()
{
	if (this != &s_systemClock)
	{
		s_systemClock.AddChild(this);
	}
}

Clock::~Clock()
{

}

void Clock::Reset()
{
	m_lastUpdateTimeInSeconds = 0.f;
	m_totalSeconds = 0.f;
	m_deltaSeconds = 0.f;
	m_frameCount = 0;

	// Get current system time
	m_lastUpdateTimeInSeconds = static_cast<float>(GetCurrentTimeSeconds());
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::UnPause()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::StepSingleFrame()
{
	m_deltaSeconds = 1.f / 60.f;
	m_isPaused = true;
}

void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}

Clock& Clock::GetSystemClock()
{
	return s_systemClock;
}

void Clock::TickSystemClock()
{
	GetSystemClock().Tick();
}

void Clock::Tick()
{
	float currentTime = static_cast<float>(GetCurrentTimeSeconds());

	if (!m_isPaused)
	{
		m_deltaSeconds = currentTime - m_lastUpdateTimeInSeconds;
		m_totalSeconds += m_deltaSeconds;
		m_frameCount++;

		// Clamp current delta seconds to max delta time
		m_deltaSeconds = GetClamped(m_deltaSeconds, 0.f, 0.1f);
	}
	else if (m_isPaused)
	{
		m_deltaSeconds *= 0.f;
	}

	m_lastUpdateTimeInSeconds = currentTime;

	// Call advance on all child clocks
	for (Clock* child : m_children)
	{
		child->Advance(m_deltaSeconds);
	}
}

void Clock::Advance(float deltaTimeSeconds)
{
	if (m_isPaused)
	{
		deltaTimeSeconds *= 0.f;
	}
	else
	{
		deltaTimeSeconds *= m_timeScale;
	}

	//DebuggerPrintf("FPS: %f TimeScale: %f\n", deltaTimeSeconds, m_timeScale);

	if (m_stepSingleFrame)
	{
		m_isPaused = true;
		m_stepSingleFrame = false;
	}

	// Update book keeping variables
	m_totalSeconds += deltaTimeSeconds;
	m_deltaSeconds = deltaTimeSeconds;
	m_frameCount++;

	// Call advance on all child clocks
	for (Clock* child : m_children)
	{
		child->Advance(deltaTimeSeconds);
	}
}

void Clock::AddChild(Clock* childClock)
{
	if (childClock != nullptr)
	{
		childClock->m_parent = this;
		m_children.emplace_back(childClock);
	}
}

void Clock::RemoveChild(Clock* childClock)
{
	// Find the clock in the list 
	std::vector<Clock*>::iterator it = std::find(m_children.begin(), m_children.end(), childClock);

	// Check if the child clock was found
	if (it != m_children.end())
	{
		(*it)->m_parent = nullptr;
		m_children.erase(it);
	}
}