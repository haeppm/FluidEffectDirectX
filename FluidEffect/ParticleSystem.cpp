#include <iostream>
#include "ParticleSystem.h"
#include "Shader.h"
#include "InputSystem.h"

ParticleSystem::ParticleSystem(DirectX::XMFLOAT3 position, ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* shaderFileName, bool hasGeometryShader)
{
	SetShader(device, deviceContext, shaderFileName, hasGeometryShader);
    m_particleSpawner = new ParticleSpawner(position, m_particles, device, deviceContext);
}

ParticleSystem::~ParticleSystem()
{
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}

	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	if (m_geometryShader)
	{
		m_geometryShader->Release();
		m_geometryShader = nullptr;
	}

	if (m_inputLayout)
	{
		m_inputLayout->Release();
		m_inputLayout = nullptr;
	}

	if (m_particleSpawner)
	{
		delete m_particleSpawner;
		m_particleSpawner = nullptr;
	}

	for (Particle* particle : m_particles)
	{
		delete particle;
	}
	m_particles.clear();
}

HRESULT ParticleSystem::Render(ID3D11DeviceContext* deviceContext, const Camera& camera)
{
	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->GSSetShader(m_geometryShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
	deviceContext->IASetInputLayout(m_inputLayout);

    for (Particle* particle : m_particles)
    {
        particle->UpdateNearestParticles(m_particles, camera);
        if(FAILED(particle->Render(deviceContext, camera)))
            return E_FAIL;
    }

    return S_OK;
}

void ParticleSystem::Update(float deltaTime)
{
	InputSystem input = InputSystem::GetInstance();

	if (input.Held(VK_SHIFT) && input.Pressed('E') && GetParticleSpawner()->m_velocity < 20.0f)
		GetParticleSpawner()->m_velocity++;
	else if (input.Pressed('E') && GetParticleSpawner()->m_spawnRate < 40.0f)
		GetParticleSpawner()->m_spawnRate++;

	if (input.Held(VK_SHIFT) && input.Pressed('Q') && GetParticleSpawner()->m_velocity > 2.0f)
		GetParticleSpawner()->m_velocity--;
	else if (input.Pressed('Q') && GetParticleSpawner()->m_spawnRate > 2.0f)
		GetParticleSpawner()->m_spawnRate--;

	if (input.Pressed('X') && GetParticleSpawner()->m_dVariance < 2.0f)
		GetParticleSpawner()->m_dVariance += 0.1f;

	if (input.Pressed('Y') && GetParticleSpawner()->m_dVariance > 0.0f)
		GetParticleSpawner()->m_dVariance -= 0.1f;


    m_particleSpawner->Update(deltaTime);

	for (int i = 0; i < m_particles.size(); i++)
	{
		m_particles[i]->Update(deltaTime);

		if (m_particles[i]->GetTimeToLive() <= 0.0f)
		{
			delete m_particles[i];
			m_particles.erase(m_particles.begin() + i);
			i--;
		}
	}
}

ParticleSpawner* ParticleSystem::GetParticleSpawner()
{
    return m_particleSpawner;
}

HRESULT ParticleSystem::SetShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* shaderFileName, bool hasGeometryShader)
{
	HRESULT hr;

	ID3D10Blob* shaderBlob = nullptr;

	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}

	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	if (m_geometryShader)
	{
		m_geometryShader->Release();
		m_geometryShader = nullptr;
	}

	if (m_inputLayout)
	{
		m_inputLayout->Release();
		m_inputLayout = nullptr;
	}

	//Compile Vertex Shader
	hr = Shader::CompileShaderFromFile(shaderFileName, "VSMain", "vs_5_0", &shaderBlob);
	if (FAILED(hr))
	{
		std::cout << "Compiling Vertex Shader failed." << std::endl;
		return hr;
	}

	hr = device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr))
	{
		std::cout << "Creating Vertex Shader failed." << std::endl;
		shaderBlob->Release();
		return hr;
	}

	//Create Input Layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = device->CreateInputLayout(layout, numElements, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &m_inputLayout);
	if (FAILED(hr))
	{
		std::cout << "Creating Input Layout failed." << std::endl;
		shaderBlob->Release();
		return hr;
	}

	shaderBlob->Release();

	//Compile Geometry Shader if used

	if (hasGeometryShader)
	{
		hr = Shader::CompileShaderFromFile(shaderFileName, "GSMain", "gs_5_0", &shaderBlob);
		if (FAILED(hr))
		{
			std::cout << "Compiling Geometry Shader failed." << std::endl;
			shaderBlob->Release();
			return hr;
		}

		hr = device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &m_geometryShader);
		if (FAILED(hr))
		{
			std::cout << "Creating Geometry Shader failed." << std::endl;
			shaderBlob->Release();
			return hr;
		}

		shaderBlob->Release();
	}
	else
	{
		m_geometryShader = nullptr;
	}

	//Compile Pixel Shader
	hr = Shader::CompileShaderFromFile(shaderFileName, "PSMain", "ps_5_0", &shaderBlob);
	if (FAILED(hr))
	{
		std::cout << "Compiling Pixel Shader failed." << std::endl;
		return hr;
	}

	hr = device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr))
	{
		std::cout << "Creating Pixel Shader failed." << std::endl;
		shaderBlob->Release();
		return hr;
	}

	shaderBlob->Release();

	return S_OK;
}
