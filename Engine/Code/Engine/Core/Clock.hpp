#pragma once
#include <vector>

class Clock
{
public:
	// Default constructor, uses the system clock as the parent of the new clock
	Clock();
	explicit Clock(Clock& parent);
	~Clock();
	Clock(const Clock& copy) = delete;

	// Reset all book keeping variables back to zero and then set the last updated time to be the current system time
	void Reset();
	bool IsPaused() const;
	void Pause();
	void UnPause();
	void TogglePause();

	// Unpause for frame then pause again the next frame
	void StepSingleFrame();

	// Set and get the value by which this clock scales delta seconds 
	void SetTimeScale(float timeScale);
	float GetTimeScale() const;
	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;
	size_t GetFrameCount() const;

public:
	// Returns a reference to a static system clock that by default will be the parent of all other clocks if a parent is not specified
	static Clock& GetSystemClock();
	// Called in begin frame to tick the system clock, which will in turn advance the system
	// clock, which will in turn advance all of its children, thus updating the entire hierarchy 
	static void TickSystemClock();

protected:
	// Calculates the current delta seconds and clamps it to the max delta time, sets the last
	// updated time, then calls advance, passing down the delta seconds 
	void Tick();

	// Calculates delta seconds based on pausing and time scale, updates all remaining book
	// keeping variables, calls advance on all child clocks and passes down our delta seconds,
	// and handles pausing after frames for stepping single frames
	void Advance(float deltaTimeSeconds);

	// Add a child clock as one of our children. Does not handle cases where the child clock already has a parent
	void AddChild(Clock* childClock);

	// Removes a child clock from our children if it is a child, otherwise does nothing 
	void RemoveChild(Clock* childClock);

protected:
	Clock* m_parent = nullptr;
	std::vector<Clock*> m_children;

	// Book keeping variables
	float m_lastUpdateTimeInSeconds = 0.f;
	float m_totalSeconds = 0.f;
	float m_deltaSeconds = 0.f;
	size_t m_frameCount = 0;

	// Time scal for this clock 
	float m_timeScale = 1.f;

	// Pauses the clock completely 
	bool m_isPaused = false;
	
	// For single stepping frames
	bool m_stepSingleFrame = false;

	// Max delta time. Useful for preventing large time steps when stepping in a debugger
	float m_maxDeltaSeconds = 0.1f;
};