#pragma once
#include <d3d11.h>
#include <vector>
#include "Camera.h"
#include "Particle.h"
#include "ParticleSpawner.h"

class ParticleSystem
{
public:
	ParticleSystem(DirectX::XMFLOAT3 position, ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* shaderFileName, bool hasGeometryShader = false);
	~ParticleSystem();
	HRESULT Render(ID3D11DeviceContext* deviceContext, const Camera& camera);
	void Update(float deltaTime);
	ParticleSpawner* GetParticleSpawner();
	HRESULT SetShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* shaderFileName, bool hasGeometryShader = false);

private:
	ParticleSpawner* m_particleSpawner;
	std::vector<Particle*> m_particles;

	ID3D11InputLayout* m_inputLayout;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
};

