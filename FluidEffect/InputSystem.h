#pragma once
#include <map>
#include <Windows.h>
#include <DirectXMath.h>
#include "KeyObserver.h"

class InputSystem
{
public:
	static InputSystem& GetInstance();
	void Initialize(HWND* hwnd);

	void ObserveKey(int key);
	void Update(float deltaTime, bool wndInFocus);

	bool Pressed(int key) { return m_keyMapper[key].m_pressed; }
	bool Held(int key) { return m_keyMapper[key].m_held; }
	bool Released(int key) { return m_keyMapper[key].m_released; }

	POINT GetMousePos() { return m_mousePos; }
	DirectX::XMFLOAT2 GetMouseVel() { return m_mouseVel; }


private:
	InputSystem() {};

	HWND* m_hwnd;
	std::map<int, KeyObserver> m_keyMapper;
	POINT m_mousePos;
	DirectX::XMFLOAT2 m_mouseVel;

};

