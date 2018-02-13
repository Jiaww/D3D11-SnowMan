#pragma once
#include "pch.h"
#include <VertexTypes.h>

struct MatrixBufferType
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX invTransWorld;
	DirectX::XMMATRIX lightView;
	DirectX::XMMATRIX lightProjection;
};

struct CameraBufferType
{
	DirectX::XMFLOAT4 camPos;
};

struct ColorBufferType
{
	DirectX::XMFLOAT4 c_color;
};

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 textureCoordinate;
};

class RModel
{
public:
	RModel();
	~RModel();

	DirectX::XMMATRIX model;
	DirectX::XMFLOAT4 color;
	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11ShaderResourceView* texture = nullptr;
	ID3D11ShaderResourceView* normalMap = nullptr;

	void setTexture(ID3D11Device1* device, const wchar_t *path);
	void setNormalMap(ID3D11Device1* device, const wchar_t *path);
};

