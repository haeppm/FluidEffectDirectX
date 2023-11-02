#include "ParticleSpawner.h"
#include "RandomValues.h"

ParticleSpawner::ParticleSpawner(DirectX::XMFLOAT3 position, std::vector<Particle*>& particleList, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	: m_particleList(particleList), m_device(device), m_deviceContext(deviceContext)
{
	m_position = position;
	m_pVariance = 0.0f;
	m_direction = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_dVariance = 0.0f;
	m_velocity = 10.0f;
	m_vVariance = 0.0f;
	m_spawnRate = 1.0f;
	m_srVariance = 0.0f;
	m_timeToLive = 1.0f;
	m_ttlVariance = 0.0f;
	m_actualSpawnTime = 0.0f;
}

void ParticleSpawner::Update(float deltaTime)
{
	m_actualSpawnTime -= deltaTime;

	if (m_actualSpawnTime <= 0.0f)
	{
		CreateParticle();
		float variance = m_srVariance * RandomValues::GetRandomValue(-1.0f, 1.0f);

		// Make sure that in m_actualSpawnTime calculation no div by ZERO happens
		if (variance <= -m_spawnRate)
			variance = -m_spawnRate + 0.01f;

		m_actualSpawnTime = 1.0f / (m_spawnRate + variance);
	}
}

void ParticleSpawner::CreateParticle()
{
	Particle* particle = new Particle(m_device, m_deviceContext);

	DirectX::XMFLOAT3 actualPosition = m_position;
	actualPosition.x += m_pVariance * RandomValues::GetRandomValue(-1.0f, 1.0f);
	actualPosition.y += m_pVariance * RandomValues::GetRandomValue(-1.0f, 1.0f);
	actualPosition.z += m_pVariance * RandomValues::GetRandomValue(-1.0f, 1.0f);
	particle->SetPosition(actualPosition);

	DirectX::XMFLOAT3 actualVelocity = m_direction;
	actualVelocity.x += m_dVariance * RandomValues::GetRandomValue(-1.0f, 1.0f);
	actualVelocity.y += m_dVariance * RandomValues::GetRandomValue(-1.0f, 1.0f);
	actualVelocity.z += m_dVariance * RandomValues::GetRandomValue(-1.0f, 1.0f);
	DirectX::XMVECTOR vActualVelocity = DirectX::XMLoadFloat3(&actualVelocity);
	vActualVelocity = DirectX::XMVector3Normalize(vActualVelocity);
	vActualVelocity = DirectX::XMVectorScale(vActualVelocity, m_velocity + m_vVariance * RandomValues::GetRandomValue(-1.0f, 1.0f));
	DirectX::XMStoreFloat3(&actualVelocity, vActualVelocity);
	particle->SetVelocity(actualVelocity);

	particle->SetTimeToLive(m_timeToLive + m_ttlVariance * RandomValues::GetRandomValue(-1.0f, 1.0f));

	m_particleList.push_back(particle);
}
