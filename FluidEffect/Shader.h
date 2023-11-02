#pragma once
#include <d3dcompiler.h>

class Shader
{
public:
	static HRESULT CompileShaderFromFile(const WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut);
};

