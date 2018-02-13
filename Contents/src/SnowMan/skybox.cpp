#include "pch.h"
#include "skybox.h"


skybox::skybox()
{
}


skybox::~skybox()
{
}

void skybox::create(ID3D11Device1* device) {
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
	std::vector<DirectX::VertexPositionNormalTexture> skybox_vertices;
	std::vector<uint16_t> skybox_indices;
	ID3D11Buffer* skybox_vertexBuffer;
	ID3D11Buffer* skybox_indexBuffer;
	DirectX::GeometricPrimitive::CreateCube(skybox_vertices, skybox_indices, 1.0);
	float epsilon = 0.001;
	// Set Texcoords
	skybox_vertices[0].textureCoordinate = DirectX::XMFLOAT2(0.5, 1.0/3.0 + epsilon);
	skybox_vertices[1].textureCoordinate = DirectX::XMFLOAT2(0.5, 2.0/3.0);
	skybox_vertices[2].textureCoordinate = DirectX::XMFLOAT2(0.25, 2.0/3.0);
	skybox_vertices[3].textureCoordinate = DirectX::XMFLOAT2(0.25, 1.0/3.0 + epsilon);

	skybox_vertices[4].textureCoordinate = DirectX::XMFLOAT2(1.0, 1.0/3.0 + epsilon);
	skybox_vertices[5].textureCoordinate = DirectX::XMFLOAT2(1.0, 2.0/3.0);
	skybox_vertices[6].textureCoordinate = DirectX::XMFLOAT2(0.75, 2.0/3.0);
	skybox_vertices[7].textureCoordinate = DirectX::XMFLOAT2(0.75, 1.0/3.0 + epsilon);

	skybox_vertices[8].textureCoordinate = DirectX::XMFLOAT2(0.75, 1.0/3.0 + epsilon);
	skybox_vertices[9].textureCoordinate = DirectX::XMFLOAT2(0.75, 2.0/3.0);
	skybox_vertices[10].textureCoordinate = DirectX::XMFLOAT2(0.5, 2.0/3.0);
	skybox_vertices[11].textureCoordinate = DirectX::XMFLOAT2(0.5, 1.0/3.0 + epsilon);

	skybox_vertices[12].textureCoordinate = DirectX::XMFLOAT2(0.25, 1.0/3.0 + epsilon);
	skybox_vertices[13].textureCoordinate = DirectX::XMFLOAT2(0.25, 2.0/3.0);
	skybox_vertices[14].textureCoordinate = DirectX::XMFLOAT2(0.0, 2.0/3.0);
	skybox_vertices[15].textureCoordinate = DirectX::XMFLOAT2(0.0, 1.0/3.0 + epsilon);

	skybox_vertices[16].textureCoordinate = DirectX::XMFLOAT2(0.25 + epsilon, 1.0/3.0);
	skybox_vertices[17].textureCoordinate = DirectX::XMFLOAT2(0.25 + epsilon, 0.0);
	skybox_vertices[18].textureCoordinate = DirectX::XMFLOAT2(0.5 - epsilon,  0.0);
	skybox_vertices[19].textureCoordinate = DirectX::XMFLOAT2(0.5 - epsilon, 1.0/3.0);

	skybox_vertices[20].textureCoordinate = DirectX::XMFLOAT2(0.5, 2.0/3.0);
	skybox_vertices[21].textureCoordinate = DirectX::XMFLOAT2(0.5, 1.0);
	skybox_vertices[22].textureCoordinate = DirectX::XMFLOAT2(0.25, 1.0);
	skybox_vertices[23].textureCoordinate = DirectX::XMFLOAT2(0.25, 2.0/3.0);

	vertexBufferDesc.ByteWidth = sizeof(DirectX::VertexPositionNormalTexture) * skybox_vertices.size();
	vertexData.pSysMem = skybox_vertices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData,
			&skybox_vertexBuffer));
	// Create Index buffer.
	indexBufferDesc.ByteWidth = sizeof(uint16_t) * skybox_indices.size();
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = skybox_indices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData,
			&skybox_indexBuffer));

	RModel* skybox = new RModel();
	skybox->vertices = skybox_vertices;
	skybox->indices = skybox_indices;
	skybox->vertexBuffer = skybox_vertexBuffer;
	skybox->indexBuffer = skybox_indexBuffer;
	skybox->model = DirectX::XMMatrixScaling(1.0, 1.0, 1.0) * DirectX::XMMatrixRotationRollPitchYaw(0.0, 0.0, 0.0) * DirectX::XMMatrixTranslation(0.0, 0.0, 0.0);
	skybox->color = DirectX::XMFLOAT4(0.7, 0.7, 0.2, 1.0);
	skybox->setTexture(device, L"Media/skybox.jpg");

	this->components.push_back(skybox);
}
