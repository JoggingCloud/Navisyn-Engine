#pragma once


class Clock;

class Timer
{
public:
	Timer();
	~Timer();
	explicit Timer(float period, const Clock* clock = nullptr);

	void Start();
	void Stop();

	float GetElapsedTime() const;
	float GetElapsedFraction() const;

	bool IsStopped() const;
	bool HasPeriodElapsed() const;
	bool DecrementPeriodIfElapsed();

public:
	const Clock* m_clock = nullptr;
	float m_startTime = 0.f;
	float m_period = 0.f;
};