#include "pch.h"
#include "plane.h"


plane::plane()
{
}


plane::~plane()
{
}

void plane::create(ID3D11Device1* device) {
	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;
	vertices.push_back(DirectX::VertexPositionNormalTexture(DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)));
	vertices.push_back(DirectX::VertexPositionNormalTexture(DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(DirectX::VertexPositionNormalTexture(DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	vertices.push_back(DirectX::VertexPositionNormalTexture(DirectX::XMFLOAT3(0.5f, 0.0f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
	D3D11_BUFFER_DESC indexBufferDesc = { 0 };
	D3D11_SUBRESOURCE_DATA indexData = { 0 };
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = 0;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	//Head
	RModel* rmodel = new RModel();
	rmodel->vertices = vertices;
	rmodel->indices = indices;
	vertexBufferDesc.ByteWidth = sizeof(DirectX::VertexPositionNormalTexture) * rmodel->vertices.size();
	vertexData.pSysMem = rmodel->vertices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData,
			&rmodel->vertexBuffer));
	// Create Index buffer.
	indexBufferDesc.ByteWidth = sizeof(uint16_t) * rmodel->indices.size();
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = rmodel->indices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData,
			&rmodel->indexBuffer));

	rmodel->model = DirectX::XMMatrixScaling(10.0, 10.0, 10.0);
	rmodel->color = DirectX::XMFLOAT4(0.9, 0.9, 0.9, 1.0);
	this->components.push_back(rmodel);
}