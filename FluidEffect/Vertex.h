#pragma once
#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 color;

	Vertex()
	{
		position = { 0.0f, 0.0f, 0.0f };
		normal = { 0.0f, 0.0f, 0.0f };
		uv = { 0.0f, 0.0f };
		color = { 1.0f, 0.0f, 0.5f };
	}

	Vertex(const DirectX::XMFLOAT3& position)
	{
		this->position = position;
		normal = { 0.0f, 0.0f, 0.0f };
		uv = { 0.0f, 0.0f };
		color = { 1.0f, 0.0f, 0.5f };
	}

	Vertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& color)
	{
		this->position = position;
		normal = { 0.0f, 0.0f, 0.0f };
		uv = { 0.0f, 0.0f };
		this->color = color;
	}
};
