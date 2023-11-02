#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include "DirectX11Helper.h"

DirectX11Helper::DirectX11Helper()
{
	m_device = nullptr;
	m_deviceContext = nullptr;
	m_swapChain = nullptr;
}

HRESULT DirectX11Helper::InitDirectX11(HWND hWnd)
{
	HRESULT hr = S_OK;

	// Create DirectX device and device context
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL featureLevel;

	// Create swap chain
	RECT rc;
	GetClientRect(hWnd, &rc);
	int windowWidth = rc.right - rc.left;
	int windowHeight = rc.bottom - rc.top;
	int msaaCount = 4;
	int msaaQuality = 0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = windowWidth;
	swapChainDesc.BufferDesc.Height = windowHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = msaaCount;
	swapChainDesc.SampleDesc.Quality = msaaQuality;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_swapChain,
		&m_device,
		&featureLevel,
		&m_deviceContext
	);

	// Create Depth Buffer
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = windowWidth;
	depthBufferDesc.Height = windowHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	depthBufferDesc.SampleDesc.Count = msaaCount;
	depthBufferDesc.SampleDesc.Quality = msaaQuality;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	m_device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthBuffer);

	//Create Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	
	//Create Depth Stencil View
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = depthBufferDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Texture2D.MipSlice = 0;
	if(m_depthBuffer)
		m_device->CreateDepthStencilView(m_depthBuffer, &dsvDesc, &m_depthStencilView);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendDesc, &m_blendState);
	m_deviceContext->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);

	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
	backBuffer->Release();
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);

	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(windowWidth);
	viewport.Height = static_cast<float>(windowHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	m_deviceContext->RSSetViewports(1, &viewport);
	return hr;
}

HRESULT DirectX11Helper::CleanUpDirectX11()
{
	HRESULT hr = S_OK;

	if (m_rasterizerState)
	{
		m_rasterizerState->Release();
		m_rasterizerState = nullptr;
	}

	if (m_swapChain)
	{
		hr = m_swapChain->SetFullscreenState(FALSE, nullptr);
		if (FAILED(hr))
			return hr;

		m_swapChain->Release();
		m_swapChain = nullptr;
	}

	if (m_blendState)
	{
		m_blendState->Release();
		m_blendState = nullptr;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = nullptr;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = nullptr;
	}

	if (m_depthBuffer)
	{
		m_depthBuffer->Release();
		m_depthBuffer = nullptr;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}

	if (m_deviceContext)
	{
		m_deviceContext->ClearState();
		m_deviceContext->Release();
		m_deviceContext = nullptr;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}

	return hr;
}

void DirectX11Helper::ClearTargetViewAndDepthBuffer()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, clearColor);
	return;
}

HRESULT DirectX11Helper::RenderObjects(std::vector<GameObject*>& gameObjects, const Camera& camera)
{
	HRESULT hr;
	for (int i = 0; i < gameObjects.size(); i++)
	{
		hr = gameObjects[i]->Render(m_deviceContext, camera);
		if (FAILED(hr))
		{
			std::cout << "Rendering of a GameObject failed." << std::endl;
			return hr;
		}
	}

	return S_OK;
}

HRESULT DirectX11Helper::RenderParticles(std::vector<ParticleSystem*>& particleSystems, const Camera& camera)
{
	HRESULT hr;
	for (int i = 0; i < particleSystems.size(); i++)
	{
		hr = particleSystems[i]->Render(m_deviceContext, camera);
		if (FAILED(hr))
		{
			std::cout << "Rendering of a GameObject failed." << std::endl;
			return hr;
		}
	}

	return S_OK;
}

ID3D11Device* DirectX11Helper::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext* DirectX11Helper::GetDeviceContext()
{
	return m_deviceContext;
}

HRESULT DirectX11Helper::DisplayFrame()
{
	return m_swapChain->Present(0, 0);
}
