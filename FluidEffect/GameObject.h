#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include "Mesh.h"
#include "Camera.h"

class GameObject
{
public:
	GameObject();

	HRESULT Render(ID3D11DeviceContext* deviceContext, const Camera& camera);

	void Update(float deltaTime);

	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetRotation(const DirectX::XMFLOAT3& rotation);
	void SetScale(const DirectX::XMFLOAT3& scale);
	void SetMesh(Mesh* mesh);

	const DirectX::XMFLOAT3& GetPosition() const;
	const DirectX::XMFLOAT3& GetRotation() const;
	const DirectX::XMFLOAT3& GetScale() const;

	Mesh& GetMesh();

	DirectX::XMMATRIX GetWorldMatrix() const;
private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;
	DirectX::XMFLOAT3 m_velocity;
	DirectX::XMFLOAT3 m_gravity = DirectX::XMFLOAT3(0.0f, -9.81f, 0.0f);

	Mesh* m_mesh;
};

