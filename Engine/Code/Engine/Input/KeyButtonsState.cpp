#include "Engine/Input/KeyButtonState.hpp"

KeyButtonState::KeyButtonState()
{

}

void KeyButtonState::Reset()
{
	if (m_isPressedThisFrame)
	{
		m_isPressedThisFrame = false;
	}
	if (m_wasPressedLastFrame)
	{
		m_wasPressedLastFrame = false;
	}
}
