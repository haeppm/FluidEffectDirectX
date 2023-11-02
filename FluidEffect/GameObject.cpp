#include "GameObject.h"

GameObject::GameObject()
{
	m_position = { 0, 0, 0 };
	m_rotation = { 0, 0, 0 };
	m_scale = { 1, 1, 1 };
	m_mesh = nullptr;
}

HRESULT GameObject::Render(ID3D11DeviceContext* deviceContext, const Camera& camera)
{
	HRESULT hr = m_mesh->Render(deviceContext, camera, GetWorldMatrix());
	return hr;
}

void GameObject::Update(float deltaTime)
{
}

void GameObject::SetPosition(const DirectX::XMFLOAT3& position)
{
	m_position = position;
}

void GameObject::SetRotation(const DirectX::XMFLOAT3& rotation)
{
	m_rotation.x = DirectX::XMConvertToRadians(rotation.x);
	m_rotation.y = DirectX::XMConvertToRadians(rotation.y);
	m_rotation.z = DirectX::XMConvertToRadians(rotation.z);
}

void GameObject::SetScale(const DirectX::XMFLOAT3& scale)
{
	m_scale = scale;
}

void GameObject::SetMesh(Mesh* mesh)
{
	m_mesh = mesh;
}

const DirectX::XMFLOAT3& GameObject::GetPosition() const
{
	return m_position;
}

const DirectX::XMFLOAT3& GameObject::GetRotation() const
{
	return m_rotation;
}

const DirectX::XMFLOAT3& GameObject::GetScale() const
{
	return m_scale;
}

Mesh& GameObject::GetMesh()
{
	return *m_mesh;
}

DirectX::XMMATRIX GameObject::GetWorldMatrix() const
{
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(m_rotation.x);
	DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationY(m_rotation.y);
	DirectX::XMMATRIX rotationZ = DirectX::XMMatrixRotationZ(m_rotation.z);
	DirectX::XMMATRIX rotation = rotationX * rotationY * rotationZ;

	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	
	return scale * rotation * translation;
}
