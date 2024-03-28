#include "Camera.h"

Camera::Camera(float screenWidth, float screenHeight, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
{
	m_position = position;
	m_rotation = rotation;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_nearPlane = 0.1f;
	m_farPlane = 1000.0f;
}

void Camera::Update(float deltaTime)
{
	InputSystem& input = InputSystem::GetInstance();
	float cameraSpeed = 20.0f;
	float cameraRotSpeed = 250.0f;

	if (input.Held('W'))
		Translate(ScaleFloat3(GetForwardVector(), cameraSpeed * deltaTime));

	if (input.Held('S'))
		Translate(ScaleFloat3(GetForwardVector(), -cameraSpeed * deltaTime));

	if (input.Held('D'))
		Translate(ScaleFloat3(GetRightVector(), cameraSpeed * deltaTime));

	if (input.Held('A'))
		Translate(ScaleFloat3(GetRightVector(), -cameraSpeed * deltaTime));

	if (input.Held(VK_RBUTTON))
		Rotate(DirectX::XMFLOAT3(input.GetMouseVel().y * cameraRotSpeed, input.GetMouseVel().x * cameraRotSpeed, 0.0f));

	if (GetRotation().x > 85.0f)
		SetRotation({ 85.0f, GetRotation().y, GetRotation().z });
	else if (GetRotation().x < -85.0f)
		SetRotation({ -85.0f, GetRotation().y, GetRotation().z });
}

void Camera::Translate(DirectX::XMFLOAT3 translation)
{
	DirectX::XMFLOAT3 newPos = m_position;
	newPos.x += translation.x;
	newPos.y += translation.y;
	newPos.z += translation.z;

	SetPosition(newPos);
}

void Camera::Rotate(DirectX::XMFLOAT3 rotationToAdd)
{
	DirectX::XMFLOAT3 rotToAddRad = DegToRad(rotationToAdd);
	DirectX::XMFLOAT3 currentRotRad = DegToRad(m_rotation);

	currentRotRad.x += rotToAddRad.x;
	currentRotRad.y += rotToAddRad.y;
	currentRotRad.z += rotToAddRad.z;

	DirectX::XMFLOAT3 newRotation = RadToDeg(currentRotRad);

	SetRotation(newRotation);
}

void Camera::SetPosition(DirectX::XMFLOAT3 position)
{
	m_position = position;
}

void Camera::SetRotation(DirectX::XMFLOAT3 rotation)
{
	m_rotation = rotation;
}

 const DirectX::XMFLOAT3& Camera::GetPosition() const
{
	return m_position;
}

 const DirectX::XMFLOAT3& Camera::GetRotation() const
 {
	 return m_rotation;
 }

 DirectX::XMMATRIX Camera::GetViewMatrix() const
 {
	 DirectX::XMMATRIX viewMatrix;
	 DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_position);
	 DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	 float pitch = DirectX::XMConvertToRadians(m_rotation.x);
	 float yaw = DirectX::XMConvertToRadians(m_rotation.y);
	 float roll = DirectX::XMConvertToRadians(m_rotation.z);
	 DirectX::XMVECTOR rot = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

	 DirectX::XMVECTOR look = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	 look = DirectX::XMVector3Rotate(look, rot);

	 DirectX::XMVECTOR target = DirectX::XMVectorAdd(pos, look);

	 viewMatrix = DirectX::XMMatrixLookAtLH(pos, target, up);
	 return viewMatrix;
 }

 DirectX::XMMATRIX Camera::GetProjectionMatrix() const
 {
	 DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60.0f), m_screenWidth / m_screenHeight, m_nearPlane, m_farPlane);
	 return projectionMatrix;
 }

 DirectX::XMFLOAT3 Camera::GetForwardVector() const
 {
	 DirectX::XMFLOAT3 rotationRad = DegToRad(m_rotation);
	 DirectX::XMVECTOR rotation = DirectX::XMLoadFloat3(&rotationRad);
	 DirectX::XMVECTOR rotationQuat = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotation);
	 DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	 forward = DirectX::XMVector3Rotate(forward, rotationQuat);

	 DirectX::XMFLOAT3 forwardFloat;
	 DirectX::XMStoreFloat3(&forwardFloat, forward);
	 return forwardFloat;
 }

 DirectX::XMFLOAT3 Camera::GetRightVector() const
 {
	 DirectX::XMFLOAT3 rotationRad = DegToRad(m_rotation);
	 DirectX::XMVECTOR rotation = DirectX::XMLoadFloat3(&rotationRad);
	 DirectX::XMVECTOR rotationQuat = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotation);
	 DirectX::XMVECTOR right = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	 right = DirectX::XMVector3Rotate(right, rotationQuat);

	 DirectX::XMFLOAT3 rightFloat;
	 DirectX::XMStoreFloat3(&rightFloat, right);
	 return rightFloat;
 }

 DirectX::XMFLOAT3 Camera::GetUpVector() const
 {
	 DirectX::XMFLOAT3 rotationRad = DegToRad(m_rotation);
	 DirectX::XMVECTOR rotation = DirectX::XMLoadFloat3(&rotationRad);
	 DirectX::XMVECTOR rotationQuat = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotation);
	 DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	 up = DirectX::XMVector3Rotate(up, rotationQuat);

	 DirectX::XMFLOAT3 upFloat;
	 DirectX::XMStoreFloat3(&upFloat, up);
	 return upFloat;
 }

 float Camera::GetScreenWidth() const
 {
	 return m_screenWidth;
 }

 float Camera::GetScreenHeight() const
 {
	 return m_screenHeight;
 }

 DirectX::XMFLOAT3 Camera::DegToRad(const DirectX::XMFLOAT3& r) const
 {
	 DirectX::XMFLOAT3 rad;
	 rad.x = DirectX::XMConvertToRadians(r.x);
	 rad.y = DirectX::XMConvertToRadians(r.y);
	 rad.z = DirectX::XMConvertToRadians(r.z);

	 return rad;
 }

 DirectX::XMFLOAT3 Camera::RadToDeg(const DirectX::XMFLOAT3& r) const
 {
	 DirectX::XMFLOAT3 deg;
	 deg.x = DirectX::XMConvertToDegrees(r.x);
	 deg.y = DirectX::XMConvertToDegrees(r.y);
	 deg.z = DirectX::XMConvertToDegrees(r.z);

	 return deg;
 }

 DirectX::XMFLOAT3 Camera::ScaleFloat3(const DirectX::XMFLOAT3& vector, float scalar)
 {
	 return DirectX::XMFLOAT3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
 }
