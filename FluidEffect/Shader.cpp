#include "Shader.h"

HRESULT Shader::CompileShaderFromFile(const WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    ID3DBlob* pErrorBlob = nullptr;
    ID3DBlob* pBlob = nullptr;
    hr = D3DCompileFromFile(fileName, nullptr, nullptr, entryPoint, shaderModel, 0, 0, &pBlob, &pErrorBlob);
    

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }

        if (pBlob)
            pBlob->Release();

        return hr;
    }

    *ppBlobOut = pBlob;

    return hr;
}
