#include "pch.h"
#include "cube.h"


cube::cube()
{
}


cube::~cube()
{
}

void cube::create(ID3D11Device1* device) {
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

	// Create Sphere data for usage
	std::vector<DirectX::VertexPositionNormalTexture> cube_vertices;
	std::vector<uint16_t> cube_indices;
	ID3D11Buffer* cube_vertexBuffer;
	ID3D11Buffer* cube_indexBuffer;
	DirectX::GeometricPrimitive::CreateCube(cube_vertices, cube_indices, 1.0);
	cube_indices = reverseIndices(cube_indices);
	vertexBufferDesc.ByteWidth = sizeof(DirectX::VertexPositionNormalTexture) * cube_vertices.size();
	vertexData.pSysMem = cube_vertices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData,
			&cube_vertexBuffer));
	// Create Index buffer.
	indexBufferDesc.ByteWidth = sizeof(uint16_t) * cube_indices.size();
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = cube_indices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData,
			&cube_indexBuffer));

	RModel* box = new RModel();
	box->vertices = cube_vertices;
	box->indices = cube_indices;
	box->vertexBuffer = cube_vertexBuffer;
	box->indexBuffer = cube_indexBuffer;
	box->model = DirectX::XMMatrixScaling(1.0, 1.0, 1.0) * DirectX::XMMatrixRotationRollPitchYaw(0.0, 0.0, 0.0) * DirectX::XMMatrixTranslation(0.0, 0.0, 0.0);
	box->color = DirectX::XMFLOAT4(0.7, 0.7, 0.2, 1.0);
	box->setTexture(device, L"Media/box.jpg");

	this->components.push_back(box);
}