#pragma once
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/JobSystem.hpp"
#include <type_traits> // for std::is_array

class DevConsole;
class MarkovSystem;

extern NamedStrings* g_defaultConfigBlackboard; // declared in EngineCommon.hpp, defined in EngineCommon.cpp
extern NamedStrings* g_gameConfigBlackboard;
extern BitmapFont* g_bitmapFont;
extern DevConsole* g_theConsole;
extern EventSystem* g_theEventSystem;
extern JobSystem* g_theJobSystem;
extern MarkovSystem* g_theMarkov;

// Base case for raw pointers
template<typename T>
inline void SafeDelete(T*& ptr)
{
	if (ptr)
	{
		delete ptr;
		ptr = nullptr;
	}
}

// For array pointers
template<typename T, size_t N>
void SafeDelete(T* (&array)[N])
{
	for (size_t i = 0; i < N; i++)
	{
		delete array[i];
		array[i] = nullptr;
	}
}

// For deleting a specific index within the array 
template<typename T , size_t N>
void SafeDelete(T* (&array)[N], size_t index)
{
	if (index < N && array[index] != nullptr)
	{
		delete array[index];
		array[index] = nullptr;
	}
}

// For vector
template <typename T>
void SafeDelete(std::vector<T*>& vec)
{
	if (!vec.empty())
	{
		for (auto item : vec)
		{
			delete item; // Delete each pointer in the vector
		}
	}
	vec.clear(); // Clear the vector
}

#define UNUSED(x) (void)(x);
#define STATIC // Does nothing; used as a CPP marker for class static data & methods
#pragma warning(disable : 26812) // Prefer enum class to enum (yes, but not always)