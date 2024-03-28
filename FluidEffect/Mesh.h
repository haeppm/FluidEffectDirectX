#pragma once
#include <d3d11.h>
#include "Camera.h"
#include "ConstantBuffer.h"

class Mesh {
public:
	virtual HRESULT Render(ID3D11DeviceContext* deviceContext, const Camera& camera, DirectX::XMMATRIX worldMatrix) = 0;
	virtual HRESULT SetShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* shaderFileName, bool hasGeometryShader) = 0;
};