#pragma once
class KeyObserver
{
public:
	KeyObserver();
	KeyObserver(int key);

	void Update();

	bool m_pressed;
	bool m_held;
	bool m_released;

private:
	int m_key;
};

