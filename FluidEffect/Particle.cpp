#include <iostream>
#include "Particle.h"
#include "ConstantBuffer.h"
#include "Vertex.h"
#include "RandomValues.h"

Particle::Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_velocity = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_timeToLive = 1.0f;
	m_nearbyParticles = NearbyParticleConstantBuffer();

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
	DirectX::XMFLOAT3 vertexColor = { 0.0f, 0.3f, 1.0f };
	DirectX::XMFLOAT3 vertexPosition = { 0.0f, 0.0f, 0.0f };

	Vertex vertexData(vertexPosition, vertexColor);
	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = &vertexData;
	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);

	D3D11_BUFFER_DESC nearbyParticleBufferDesc;
	ZeroMemory(&nearbyParticleBufferDesc, sizeof(D3D11_BUFFER_DESC));
	nearbyParticleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	nearbyParticleBufferDesc.ByteWidth = sizeof(NearbyParticleConstantBuffer);
	nearbyParticleBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	nearbyParticleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	nearbyParticleBufferDesc.MiscFlags = 0;
	nearbyParticleBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&nearbyParticleBufferDesc, NULL, &m_nearbyParticleBuffer);
}

Particle::~Particle()
{
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	if (m_nearbyParticleBuffer)
	{
		m_nearbyParticleBuffer->Release();
		m_nearbyParticleBuffer = nullptr;
	}

	if (m_constantBuffer)
	{
		m_constantBuffer->Release();
		m_constantBuffer = nullptr;
	}
}

HRESULT Particle::Render(ID3D11DeviceContext* deviceContext, const Camera& camera)
{
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	
	D3D11_MAPPED_SUBRESOURCE constantBufferSR;
	if (SUCCEEDED(deviceContext->Map(m_constantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &constantBufferSR)))
	{
		ConstantBuffer constantBuffer;
		constantBuffer.world = worldMatrix;
		constantBuffer.view = camera.GetViewMatrix();
		constantBuffer.projection = camera.GetProjectionMatrix();
		constantBuffer.worldView = constantBuffer.world * constantBuffer.view;
		constantBuffer.worldViewProj = constantBuffer.worldView * constantBuffer.projection;
		constantBuffer.inverseWorld = DirectX::XMMatrixInverse(nullptr, constantBuffer.world);
		constantBuffer.inverseView = DirectX::XMMatrixInverse(nullptr, constantBuffer.view);
		constantBuffer.inverseProjection = DirectX::XMMatrixInverse(nullptr, constantBuffer.projection);
		memcpy(constantBufferSR.pData, &constantBuffer, sizeof(ConstantBuffer));
		deviceContext->Unmap(m_constantBuffer, NULL);	
	}

	D3D11_MAPPED_SUBRESOURCE nearbyParticleBufferSR;
	if (SUCCEEDED(deviceContext->Map(m_nearbyParticleBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &nearbyParticleBufferSR)))
	{
		memcpy(nearbyParticleBufferSR.pData, &m_nearbyParticles, sizeof(NearbyParticleConstantBuffer));
		deviceContext->Unmap(m_nearbyParticleBuffer, NULL);
	}
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	deviceContext->GSSetConstantBuffers(0, 1, &m_constantBuffer);

	deviceContext->PSSetConstantBuffers(1, 1, &m_nearbyParticleBuffer);
	
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->Draw(1, 0);

	return S_OK;
}

void Particle::Update(float deltaTime)
{
    DirectX::XMVECTOR vVelocity = DirectX::XMLoadFloat3(&m_velocity);
    DirectX::XMVECTOR vGravity = DirectX::XMLoadFloat3(&m_gravity);
	DirectX::XMVECTOR vDrag = DirectX::XMVectorScale(vVelocity, 0.1f);
    vVelocity = DirectX::XMVectorAdd(vVelocity, DirectX::XMVectorScale(vGravity, deltaTime));
    vVelocity = DirectX::XMVectorSubtract(vVelocity, DirectX::XMVectorScale(vDrag, deltaTime));
    DirectX::XMStoreFloat3(&m_velocity, vVelocity);
	
    DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&m_position);
    vPosition = DirectX::XMVectorAdd(vPosition, DirectX::XMVectorScale(vVelocity, deltaTime));
    DirectX::XMStoreFloat3(&m_position, vPosition);
	
	m_timeToLive -= deltaTime;
}

void Particle::CalculateNearestParticles(std::vector<Particle*>& particles, const Camera& camera)
{
	m_nearbyParticles = NearbyParticleConstantBuffer();
	DirectX::XMVECTOR thisPos = DirectX::XMLoadFloat3(&m_position);

	for (int i = 0; i < particles.size(); i++)
	{
		DirectX::XMVECTOR otherPos = DirectX::XMLoadFloat3(&particles[i]->m_position);		

		if (i < MAX_NEARBY_PARTICLES)	// Just set the Particle positions until every value in the Array has been set once
		{
			m_nearbyParticles.particlePos[i] = DirectX::XMFLOAT4(particles[i]->m_position.x, particles[i]->m_position.y, particles[i]->m_position.z, 1.0f);
		}
		else							// Then swap the furthest particle position with the current one if it is closer
		{
			int indexFurthest = 0;
			float distToFurthestSq = 0.0f;
			
			// Calculate what the index of the currently furthest away position is
			for (int k = 0; k < MAX_NEARBY_PARTICLES; k++)
			{
				DirectX::XMVECTOR furthestPos = DirectX::XMLoadFloat4(&m_nearbyParticles.particlePos[indexFurthest]);
				DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat4(&m_nearbyParticles.particlePos[k]);
				distToFurthestSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(furthestPos, thisPos)));
				float distToCurrentSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(currentPos, thisPos)));

				if (distToCurrentSq > distToFurthestSq)
					indexFurthest = k;
			}

			// Swap furthest position if current Particle is closer
			float distToOtherSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(otherPos, thisPos)));
			if (distToOtherSq < distToFurthestSq)
			{
				m_nearbyParticles.particlePos[indexFurthest] = DirectX::XMFLOAT4(particles[i]->m_position.x, particles[i]->m_position.y, particles[i]->m_position.z, 1.0f);
			}
		}
	}

	// Transform nearby particles into view space
	for (int i = 0; i < MAX_NEARBY_PARTICLES; i++)
	{
		DirectX::XMVECTOR worldPos = DirectX::XMLoadFloat4(&m_nearbyParticles.particlePos[i]);
		DirectX::XMVECTOR viewPos = DirectX::XMVector4Transform(worldPos, camera.GetViewMatrix());
		DirectX::XMStoreFloat4(&m_nearbyParticles.particlePos[i], viewPos);
	}
}

void Particle::SetPosition(DirectX::XMFLOAT3 position)
{
	m_position = position;
}

const DirectX::XMFLOAT3& Particle::GetPosition() const
{
	return m_position;
}

void Particle::SetVelocity(DirectX::XMFLOAT3 velocity)
{
	m_velocity = velocity;
}

DirectX::XMFLOAT3 Particle::GetVelocity()
{
	return m_velocity;
}

void Particle::SetTimeToLive(float ttl)
{
	m_timeToLive = ttl;
}

float Particle::GetTimeToLive()
{
	return m_timeToLive;
}
