#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <DirectXMath.h>
#include "StaticMesh.h"
#include "Shader.h"
#include "PointLight.h"

StaticMesh::StaticMesh(std::string filename, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	LoadObjFile(filename, device, deviceContext);
	SetShader(device, deviceContext, L"DefaultShader.hlsl", false);
}

StaticMesh::StaticMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_constantBuffer = nullptr;
	m_geometryShader = nullptr;
	m_inputLayout = nullptr;
	m_pixelShader = nullptr;
	m_vertexShader = nullptr;

	float sideHalfLength = 0.6f;
	m_numVertices = 3;
	m_vertices = new Vertex[m_numVertices]
	{
		{{-sideHalfLength, -sideHalfLength, sideHalfLength}},
		{{0.0f, sideHalfLength, sideHalfLength}},
		{{sideHalfLength, -sideHalfLength, sideHalfLength}}
	};

	m_numIndices = 3;
	m_indices = new unsigned int[m_numIndices]
	{
		0, 2, 1
	};

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = m_vertices;
	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);

	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * m_numIndices;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = m_indices;
	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);

	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	SetShader(device, deviceContext, L"DefaultShader.hlsl", false);
}

StaticMesh::~StaticMesh()
{
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	if (m_constantBuffer)
	{
		m_constantBuffer->Release();
		m_constantBuffer = nullptr;
	}

	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}

	if (m_geometryShader)
	{
		m_geometryShader->Release();
		m_geometryShader = nullptr;
	}

	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	if (m_inputLayout)
	{
		m_inputLayout->Release();
		m_inputLayout = nullptr;
	}
	delete[] m_indices;
	delete[] m_vertices;
}

HRESULT StaticMesh::Render(ID3D11DeviceContext* deviceContext, const Camera& camera, DirectX::XMMATRIX worldMatrix)
{
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
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->GSSetShader(m_geometryShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
	deviceContext->IASetInputLayout(m_inputLayout);

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &m_constantBuffer);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->DrawIndexed(m_numIndices, 0, 0);

	return S_OK;
}

HRESULT StaticMesh::SetShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* shaderFileName, bool hasGeometryShader)
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

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);
	if (FAILED(hr))
	{
		std::cout << "Creating Constant Buffer failed." << std::endl;
		return hr;
	}

	return S_OK;
}

void StaticMesh::SetColor(ID3D11Device* device, const DirectX::XMFLOAT3& color)
{
	for (int i = 0; i < m_numVertices; i++)
	{
		m_vertices[i].color = color;
	}

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_numVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = m_vertices;
	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);
}

void StaticMesh::LoadObjFile(std::string filename, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	std::ifstream objFile(filename);

	if (!objFile.is_open())
	{
		std::cerr << "Failed to open file: " << filename << std::endl;
		return;
	}

	std::vector<DirectX::XMFLOAT3> tempPositions;
	std::vector<DirectX::XMFLOAT3> tempNormals;
	std::vector<DirectX::XMFLOAT2> tempUV;
	std::vector<int> tempVIndices;
	std::vector<int> tempVnIndices;
	std::vector<int> tempUvIndices;

	std::string line;
	while (std::getline(objFile, line))
	{
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "v")
		{
			float x, y, z;
			iss >> x >> y >> z;
			tempPositions.push_back({ x, y, z });
		}
		else if (token == "vn")
		{
			float x, y, z;
			iss >> x >> y >> z;
			tempNormals.push_back({ x, y, z });
		}
		else if (token == "vt")
		{
			float x, y;
			iss >> x >> y;
			tempUV.push_back({ x, y });
		}
		else if (token == "f")
		{
			const int verticesPerFace = 3;
			int iV[verticesPerFace] = { -1, -1, -1 };
			int iUV[verticesPerFace] = { -1,  -1, -1 };
			int iVN[verticesPerFace] = { -1, -1, -1 };

			unsigned int x = 0;
			std::string faceVertexData;

			// Extract the VertexIds, UVIds and VertexNormalIds from each of
			// the 3 Vertex Blocks that make up a face (f iV1/iUV1/iVN1 iV2/iUV2/iVN2 iV3/iUV3/iVN3)
			while (iss >> faceVertexData)					
			{														
				if (x >= verticesPerFace)
					break;

				std::istringstream issDataBlock(faceVertexData);
				char delimiter;

				issDataBlock >> iV[x] >> delimiter;

				if (std::isdigit(issDataBlock.peek()))
				{
					issDataBlock >> iUV[x] >> delimiter >> iVN[x];
				}
				else
				{
					issDataBlock >> delimiter >> iVN[x];
				}

				tempVIndices.push_back(iV[x] - 1);		// -1 because .obj files start at index 1 and not 0
				tempUvIndices.push_back(iUV[x] - 1);
				tempVnIndices.push_back(iVN[x] - 1);
				
				x++;
			}
		}
	}
	objFile.close();

	

	m_numVertices = tempPositions.size();
	m_numIndices = tempVIndices.size();

	std::vector<DirectX::XMFLOAT3> smoothedNormals;

	GetSmoothedNormals(m_numVertices, tempVIndices, tempVnIndices, tempNormals, smoothedNormals);

	m_vertices = new Vertex[m_numVertices];
	for (int i = 0; i < m_numVertices; i++)
	{
		m_vertices[i].position = tempPositions[i];
		m_vertices[i].normal = smoothedNormals[i];
		//m_vertices[i].uv
	}

	m_indices = new unsigned int[m_numIndices];
	for (int i = 0; i < m_numIndices; i++)
	{
		m_indices[i] = tempVIndices[i];
	}

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_numVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = m_vertices;
	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * m_numIndices;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = m_indices;
	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);
}

void StaticMesh::GetSmoothedNormals(int numVertices, const std::vector<int>& vertexIndices, const std::vector<int>& normalIndices, const std::vector<DirectX::XMFLOAT3>& normals, std::vector<DirectX::XMFLOAT3>& out)
{
	out.clear();
	out.resize(numVertices);

	for (int i = 0; i < vertexIndices.size(); i++)
	{
		int currentVertexId = vertexIndices[i];
		int currentNormalId = normalIndices[i];

		if (currentNormalId < 0)
			return;

		out[currentVertexId].x += normals[currentNormalId].x;
		out[currentVertexId].y += normals[currentNormalId].y;
		out[currentVertexId].z += normals[currentNormalId].z;
	}

	for (DirectX::XMFLOAT3& n : out)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&n);
		v = DirectX::XMVector3Normalize(v);
		DirectX::XMStoreFloat3(&n, v);
	}
}
