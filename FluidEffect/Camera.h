#pragma once
#include <DirectXMath.h>
#include "InputSystem.h"

class Camera
{
public:
	Camera(float screenWidth, float screenHeight, DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT3 rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	
	void Update(float deltaTime);

	void Translate(DirectX::XMFLOAT3 translation);
	void Rotate(DirectX::XMFLOAT3 rotationToAdd);

	void SetPosition(DirectX::XMFLOAT3 position);
	void SetRotation(DirectX::XMFLOAT3 rotation);

	const DirectX::XMFLOAT3& GetPosition() const;
	const DirectX::XMFLOAT3& GetRotation() const;

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;

	DirectX::XMFLOAT3 GetForwardVector() const;
	DirectX::XMFLOAT3 GetRightVector() const;
	DirectX::XMFLOAT3 GetUpVector() const;

	float GetScreenWidth() const;
	float GetScreenHeight() const;

private:
	DirectX::XMFLOAT3 DegToRad(const DirectX::XMFLOAT3& r) const;
	DirectX::XMFLOAT3 RadToDeg(const DirectX::XMFLOAT3& r) const;

	DirectX::XMFLOAT3 ScaleFloat3(const DirectX::XMFLOAT3& vector, float scalar);

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	float m_screenWidth;
	float m_screenHeight;
	float m_nearPlane;
	float m_farPlane;
};

