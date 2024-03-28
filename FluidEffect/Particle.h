#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include "Camera.h"

const int MAX_NEARBY_PARTICLES = 32;

struct alignas(16) NearbyParticleConstantBuffer
{
	DirectX::XMFLOAT4 particlePos[MAX_NEARBY_PARTICLES] ;
};

class Particle
{
public:
	Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~Particle();

	HRESULT Render(ID3D11DeviceContext* deviceContext, const Camera& camera);
	void Update(float deltaTime);

	void UpdateNearestParticles(std::vector<Particle*>& particles, const Camera& camera);

	void SetPosition(DirectX::XMFLOAT3 position);
	const DirectX::XMFLOAT3& GetPosition() const;

	void SetVelocity(DirectX::XMFLOAT3 velocity);
	DirectX::XMFLOAT3 GetVelocity();

	void SetTimeToLive(float ttl);
	float GetTimeToLive();

private:
	DirectX::XMFLOAT3 m_velocity;
	DirectX::XMFLOAT3 m_position;
	float m_timeToLive;
	NearbyParticleConstantBuffer m_nearbyParticles;

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_constantBuffer;
	ID3D11Buffer* m_nearbyParticleBuffer;

	const DirectX::XMFLOAT3 m_gravity = { 0.0f, -9.81f, 0.0f };
};

