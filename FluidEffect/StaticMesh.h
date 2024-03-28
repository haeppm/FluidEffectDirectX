#pragma once
#include <String>
#include <d3d11.h>
#include "Mesh.h"
#include "Vertex.h"

class StaticMesh : public Mesh
{
public:
	StaticMesh(std::string filename, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	StaticMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~StaticMesh();

	HRESULT Render(ID3D11DeviceContext* deviceContext, const Camera& camera, DirectX::XMMATRIX worldMatrix) override;

	int GetNumVertices() { return m_numVertices; }
	int GetNumFaces() { return m_numIndices; }
	ID3D11Buffer* GetVertexBuffer() { return m_vertexBuffer; }
	UINT* GetIndexBuffer() { return m_indices; }

	HRESULT SetShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* shaderFileName, bool hasGeometryShader) override;
	void SetColor(ID3D11Device* device, const DirectX::XMFLOAT3& color);

private:
	void LoadObjFile(std::string filename, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void GetSmoothedNormals(int numVertices, const std::vector<int>& vertexIndices, const std::vector<int>& normalIndices, const std::vector<DirectX::XMFLOAT3>& normals, std::vector<DirectX::XMFLOAT3> &out);

	Vertex* m_vertices;
	int m_numVertices;

	unsigned int* m_indices;
	int m_numIndices;

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	ID3D11Buffer* m_constantBuffer;

	ID3D11InputLayout* m_inputLayout;
	ID3D11VertexShader* m_vertexShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11PixelShader* m_pixelShader;
};

