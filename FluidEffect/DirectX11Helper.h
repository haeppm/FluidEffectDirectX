#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include "GameObject.h"
#include "ParticleSystem.h"
#include "Camera.h"

class DirectX11Helper
{
public:
	DirectX11Helper();
	HRESULT InitDirectX11(HWND hWnd);
	HRESULT CleanUpDirectX11();
	void ClearTargetViewAndDepthBuffer();
	HRESULT RenderObjects(std::vector<GameObject*>& gameObjects, const Camera& camera);
	HRESULT RenderParticles(std::vector<ParticleSystem*>& particleSystems, const Camera& camera);
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	HRESULT DisplayFrame();

private:
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	IDXGISwapChain* m_swapChain;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11Texture2D* m_depthBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterizerState;
	ID3D11BlendState* m_blendState;
};

