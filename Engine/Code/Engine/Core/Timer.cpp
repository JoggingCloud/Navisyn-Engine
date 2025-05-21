#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"

Timer::~Timer()
{
}

Timer::Timer(float period, const Clock* clock)
	: m_period(period), m_clock(clock)
{
	if (m_clock == nullptr)
	{
		m_clock = &Clock::GetSystemClock();
	}
}

Timer::Timer()
{
	m_clock = &Clock::GetSystemClock();
}

void Timer::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
}

void Timer::Stop()
{
	m_startTime = 0.f;
}

float Timer::GetElapsedTime() const
{
	if (m_clock->IsPaused())
	{
		return 0.f;
	}
	else
	{
		return m_clock->GetTotalSeconds() - m_startTime;
	}
}

float Timer::GetElapsedFraction() const
{
	return GetElapsedTime() / m_period;
}

bool Timer::IsStopped() const
{
	if (m_startTime == 0.f)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Timer::HasPeriodElapsed() const
{
	if (GetElapsedTime() > m_period && !m_clock->IsPaused())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Timer::DecrementPeriodIfElapsed()
{
	if (HasPeriodElapsed())
	{
		m_startTime += m_period;
		return true;
	}
	else
	{
		return false;
	}
}
