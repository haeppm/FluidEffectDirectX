#include <Windows.h>
#include "KeyObserver.h"

KeyObserver::KeyObserver() : KeyObserver(-1)
{
}

KeyObserver::KeyObserver(int key)
{
	m_key = key;
	m_pressed = false;
	m_held = false;
	m_released = false;
}

void KeyObserver::Update()
{
	bool isKeyDown = (GetAsyncKeyState(m_key) & 0x8000) != 0;

	m_pressed = isKeyDown && !m_held;
	m_released = !isKeyDown && m_held;
	m_held = isKeyDown;
}
