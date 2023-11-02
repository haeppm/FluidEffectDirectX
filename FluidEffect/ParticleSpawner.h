#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Particle.h"

class ParticleSpawner
{
public:
	ParticleSpawner(DirectX::XMFLOAT3 position, std::vector<Particle*>& particleList, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void Update(float deltaTime);

private:
	void CreateParticle();

public:
	DirectX::XMFLOAT3 m_position;
	float m_pVariance;
	DirectX::XMFLOAT3 m_direction;
	float m_dVariance;
	float m_velocity;
	float m_vVariance;
	float m_spawnRate;
	float m_srVariance;
	float m_timeToLive;
	float m_ttlVariance;

private:
	std::vector<Particle*>& m_particleList;
	float m_actualSpawnTime;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
};

