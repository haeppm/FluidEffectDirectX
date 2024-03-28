#pragma once
#include <DirectXMath.h>

struct alignas(16) ConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX worldView;
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMMATRIX inverseWorld;
	DirectX::XMMATRIX inverseView;
	DirectX::XMMATRIX inverseProjection;
};