#include <Windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include "DirectX11Helper.h"
#include "Mesh.h"
#include "StaticMesh.h"
#include "GameObject.h"
#include "Camera.h"
#include "Particle.h"
#include "InputSystem.h"
#include "PointLight.h"

bool wndInFocus = true;

void SetupLightsShaderResource(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int maxNumOfLights, PointLight* lights, const Camera& camera);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SETFOCUS:
			wndInFocus = true;
		break;

	case WM_KILLFOCUS:
		wndInFocus = false;
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) __checkReturn
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"MyWindowClass";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
		return 1;

	RECT rc = { 0, 0, 1920, 1080 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindow(
		L"MyWindowClass",
		L"MyWindow",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
		return 1;


	DirectX11Helper dxHelper = DirectX11Helper();
	if(FAILED(dxHelper.InitDirectX11(hWnd)))
		return 1;

	InputSystem& input = InputSystem::GetInstance();
	input.Initialize(&hWnd);
	input.ObserveKey('W');
	input.ObserveKey('S');
	input.ObserveKey('A');
	input.ObserveKey('D');
	input.ObserveKey('Q');
	input.ObserveKey('E');
	input.ObserveKey('X');
	input.ObserveKey('Y');
	input.ObserveKey('1');
	input.ObserveKey('2');
	input.ObserveKey('3');
	input.ObserveKey(VK_RBUTTON);
	input.ObserveKey(VK_SHIFT);

	Camera camera = Camera(static_cast<float>(rc.right - rc.left), static_cast<float>(rc.bottom - rc.top), { 0.0f, 5.0f, -15.0f }, {0.0f, 0.0f, 0.0f});

	const int maxNumOfLights = 5;
	PointLight lights[maxNumOfLights] = {
		{{7.0f, 2.0f, -5.0f}, {1.0f, 1.0f, 0.7f}, 10.0f},
		{{-7.0f, 5.0f, -5.0f}, {1.0f, 1.0f, 0.7f}, 4.0f},
		{{0.0f, 15.0f, 0.0f}, {1.0f, 1.0f, 0.7f}, 5.0f},
	};
	SetupLightsShaderResource(dxHelper.GetDevice(), dxHelper.GetDeviceContext(), maxNumOfLights, lights, camera);

	StaticMesh floorMesh = StaticMesh("Floor.obj", dxHelper.GetDevice(), dxHelper.GetDeviceContext());
	floorMesh.SetColor(dxHelper.GetDevice(), { 0.2f, 0.2f, 0.2f });
	floorMesh.SetShader(dxHelper.GetDevice(), dxHelper.GetDeviceContext(), L"BlinnPhongShader.hlsl", false);
	GameObject floorObj;
	floorObj.SetMesh(&floorMesh);
	floorObj.SetPosition({ 0.0f, 0.0f, 0.0f });
	floorObj.SetRotation({ 0.0f, 90.0f, 0.0f });

	StaticMesh pipeMesh = StaticMesh("Pipe.obj", dxHelper.GetDevice(), dxHelper.GetDeviceContext());
	pipeMesh.SetColor(dxHelper.GetDevice(), { 0.8f, 0.4f, 0.2f });
	pipeMesh.SetShader(dxHelper.GetDevice(), dxHelper.GetDeviceContext(), L"BlinnPhongShader.hlsl", false);
	GameObject pipeObj;
	pipeObj.SetMesh(&pipeMesh);
	pipeObj.SetPosition({ -8.0f, 0.0f, 0.0f });
	pipeObj.SetRotation({ 0.0f, -90.0f, 0.0f });

	std::vector<GameObject*> gameObjectList;
	gameObjectList.push_back(&floorObj);
	gameObjectList.push_back(&pipeObj);

	ParticleSystem particleSystem = ParticleSystem({ -6.0f, 7.5f, 0.0f }, dxHelper.GetDevice(), dxHelper.GetDeviceContext(), L"GooShader.hlsl", true);
	particleSystem.GetParticleSpawner()->m_timeToLive = 1.5f;
	particleSystem.GetParticleSpawner()->m_spawnRate = 6;
	particleSystem.GetParticleSpawner()->m_srVariance = 0.5f;
	particleSystem.GetParticleSpawner()->m_direction = { 1.0f, 0.25f, 0.0f };
	particleSystem.GetParticleSpawner()->m_dVariance = 0.1f;
	particleSystem.GetParticleSpawner()->m_vVariance = 0.3f;
	particleSystem.GetParticleSpawner()->m_velocity = 7.0f;

	std::vector<ParticleSystem*> particleSystemList;
	particleSystemList.push_back(&particleSystem);

	

	ShowWindow(hWnd, nCmdShow);

	bool bGotMsg;
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	HRESULT hr = S_OK;
	float deltaTime = 0.0f;
	while (msg.message != WM_QUIT || FAILED(hr))
	{

		auto startTime = std::chrono::high_resolution_clock::now();

		bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0;
		if (bGotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		input.Update(deltaTime, wndInFocus);

		if (input.Pressed('1'))
			particleSystem.SetShader(dxHelper.GetDevice(), dxHelper.GetDeviceContext(), L"PointShader.hlsl", true);

		if (input.Pressed('2'))
			particleSystem.SetShader(dxHelper.GetDevice(), dxHelper.GetDeviceContext(), L"QuadShader.hlsl", true);

		if (input.Pressed('3'))
			particleSystem.SetShader(dxHelper.GetDevice(), dxHelper.GetDeviceContext(), L"GooShader.hlsl", true);

		camera.Update(deltaTime);

		for (GameObject* gameObject : gameObjectList)
			gameObject->Update(deltaTime);


		for (ParticleSystem* particleSystem : particleSystemList)
			particleSystem->Update(deltaTime);

		dxHelper.ClearTargetViewAndDepthBuffer();
		hr = dxHelper.RenderObjects(gameObjectList, camera);
		hr = dxHelper.RenderParticles(particleSystemList, camera);
		dxHelper.DisplayFrame();

		auto endTime = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime).count();
	}
	return dxHelper.CleanUpDirectX11();
}

void SetupLightsShaderResource(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int maxNumOfLights, PointLight* lights, const Camera& camera)
{
	D3D11_BUFFER_DESC pointLightBufferDesc;
	ZeroMemory(&pointLightBufferDesc, sizeof(D3D11_BUFFER_DESC));
	pointLightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pointLightBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	pointLightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pointLightBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pointLightBufferDesc.StructureByteStride = sizeof(PointLight);
	pointLightBufferDesc.ByteWidth = sizeof(PointLight) * maxNumOfLights;

	ID3D11Buffer* pointLightBuffer;
	device->CreateBuffer(&pointLightBufferDesc, nullptr, &pointLightBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxNumOfLights;

	ID3D11ShaderResourceView* pointLightSRV;
	device->CreateShaderResourceView(pointLightBuffer, &srvDesc, &pointLightSRV);

	D3D11_MAPPED_SUBRESOURCE pointLightBufferSR;
	if (SUCCEEDED(deviceContext->Map(pointLightBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &pointLightBufferSR)))
	{
		PointLight* pointLightDataPtr = static_cast<PointLight*>(pointLightBufferSR.pData);

		for (int i = 0; i < maxNumOfLights; i++)
		{
			pointLightDataPtr[i] = lights[i];
		}
		deviceContext->Unmap(pointLightBuffer, NULL);
	}

	deviceContext->PSSetShaderResources(0, 1, &pointLightSRV);
}