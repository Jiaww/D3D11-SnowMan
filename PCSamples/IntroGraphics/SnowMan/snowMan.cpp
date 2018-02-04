#include "pch.h"
#include "snowMan.h"


snowMan::snowMan()
{
}

snowMan::~snowMan()
{
}

void snowMan::create(ID3D11Device1* device) {
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
	std::vector<DirectX::VertexPositionNormalTexture> sphere_vertices;
	std::vector<uint16_t> sphere_indices;
	ID3D11Buffer* sphere_vertexBuffer;
	ID3D11Buffer* sphere_indexBuffer;
	DirectX::GeometricPrimitive::CreateSphere(sphere_vertices, sphere_indices, 1.0);
	sphere_indices = reverseIndices(sphere_indices);
	vertexBufferDesc.ByteWidth = sizeof(DirectX::VertexPositionNormalTexture) * sphere_vertices.size();
	vertexData.pSysMem = sphere_vertices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData,
			&sphere_vertexBuffer));
	// Create Index buffer.
	indexBufferDesc.ByteWidth = sizeof(uint16_t) * sphere_indices.size();
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = sphere_indices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData,
			&sphere_indexBuffer));

	// Create Cone Data for usage
	std::vector<DirectX::VertexPositionNormalTexture> cone_vertices;
	std::vector<uint16_t> cone_indices;
	ID3D11Buffer* cone_vertexBuffer;
	ID3D11Buffer* cone_indexBuffer;
	DirectX::GeometricPrimitive::CreateCone(cone_vertices, cone_indices);
	cone_indices = reverseIndices(cone_indices);
	vertexBufferDesc.ByteWidth = sizeof(DirectX::VertexPositionNormalTexture) * cone_vertices.size();
	vertexData.pSysMem = cone_vertices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData,
			&cone_vertexBuffer));
	// Create Index buffer.
	indexBufferDesc.ByteWidth = sizeof(uint16_t) * cone_indices.size();
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = cone_indices.data();
	DX::ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData,
			&cone_indexBuffer));

	//Head(sphere, )
	RModel* head = new RModel();
	head->vertices = sphere_vertices;
	head->indices = sphere_indices;
	head->vertexBuffer = sphere_vertexBuffer;
	head->indexBuffer = sphere_indexBuffer;
	head->model = DirectX::XMMatrixScaling(0.6, 0.6, 0.6) * DirectX::XMMatrixTranslation(0.0, 1.25, 0.0);
	head->color = DirectX::XMFLOAT4(0.9, 0.7, 0.4, 1.0);
	this->components.push_back(head);

	// Body(sphere)
	RModel* body = new RModel();
	body->vertices = sphere_vertices;
	body->indices = sphere_indices;
	body->vertexBuffer = sphere_vertexBuffer;
	body->indexBuffer = sphere_indexBuffer;
	body->model = DirectX::XMMatrixScaling(1.0, 1.1, 1.0) * DirectX::XMMatrixTranslation(0.0, 0.55, 0.0);
	body->color = DirectX::XMFLOAT4(0.9, 0.7, 0.4, 1.0);
	this->components.push_back(body);

	//Left Eye (sphere)
	RModel* leftEye = new RModel();
	leftEye->vertices = sphere_vertices;
	leftEye->indices = sphere_indices;
	leftEye->vertexBuffer = sphere_vertexBuffer;
	leftEye->indexBuffer = sphere_indexBuffer;
	leftEye->model = DirectX::XMMatrixScaling(0.1, 0.1, 0.1) * DirectX::XMMatrixTranslation(-0.11, 1.37, -0.23);
	leftEye->color = DirectX::XMFLOAT4(0.1, 0.1, 0.1, 1.0);
	this->components.push_back(leftEye);
	//Right Eye (sphere)
	RModel* rightEye = new RModel();
	rightEye->vertices = sphere_vertices;
	rightEye->indices = sphere_indices;
	rightEye->vertexBuffer = sphere_vertexBuffer;
	rightEye->indexBuffer = sphere_indexBuffer;
	rightEye->model = DirectX::XMMatrixScaling(0.1, 0.1, 0.1) * DirectX::XMMatrixTranslation(0.11, 1.37, -0.23);
	rightEye->color = DirectX::XMFLOAT4(0.1, 0.1, 0.1, 1.0);
	this->components.push_back(rightEye);

	//Nose (Cone)
	RModel* nose = new RModel();
	nose->vertices = cone_vertices;
	nose->indices = cone_indices;
	nose->vertexBuffer = cone_vertexBuffer;
	nose->indexBuffer = cone_indexBuffer;
	nose->model = DirectX::XMMatrixScaling(0.2, 0.4, 0.2) * DirectX::XMMatrixRotationRollPitchYaw(-DirectX::XM_PI*0.5, 0.0, 0.0) * DirectX::XMMatrixTranslation(0.0, 1.25, -0.29);
	nose->color = DirectX::XMFLOAT4(0.85, 0.2, 0.2, 1.0);
	this->components.push_back(nose);

}