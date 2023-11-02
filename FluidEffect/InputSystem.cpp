#include "InputSystem.h"

InputSystem& InputSystem::GetInstance()
{
	static InputSystem input;
	return input;
}

void InputSystem::Initialize(HWND* hwnd)
{
	m_hwnd = hwnd;
}

void InputSystem::ObserveKey(int key)
{
	m_keyMapper[key] = KeyObserver(key);
}

void InputSystem::Update(float deltaTime, bool wndInFocus)
{
	if (wndInFocus)
	{
		for (auto& key : m_keyMapper)
		{
			key.second.Update();
		}

		POINT currentMousePos;
		GetCursorPos(&currentMousePos);
		ScreenToClient(*m_hwnd, &currentMousePos);

		RECT clientRect;
		GetClientRect(*m_hwnd, &clientRect);

		int windowWidth = clientRect.right - clientRect.left;
		int windowHeight = clientRect.bottom - clientRect.top;

		int mouseDeltaX = currentMousePos.x - m_mousePos.x;
		int mouseDeltaY = currentMousePos.y - m_mousePos.y;

		m_mouseVel.x = static_cast<float>(mouseDeltaX) / static_cast<float>(windowWidth);
		m_mouseVel.y = static_cast<float>(mouseDeltaY) / static_cast<float>(windowHeight);

		m_mousePos = currentMousePos;
	}
	else
	{
		for (auto& key : m_keyMapper)
		{
			key.second.m_pressed = false;
			key.second.m_held = false;
			key.second.m_released = false;
		}
	}
}
