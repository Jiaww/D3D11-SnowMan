#include "pch.h"
#include "terrain.h"

terrain::terrain()
{
}


terrain::~terrain()
{
}

void terrain::create(ID3D11Device1* device) {
	int size = 256 * 256;
	float* mapHeights = new float[size];
	wchar_t buff[MAX_PATH];
	DX::FindMediaFile(buff, MAX_PATH, HP_filename);
	_bstr_t b(buff);
	char* c = b;
	FILE *fp = fopen(c, "r");
	uint16_t *heightsBuffer = new uint16_t[size];
	fread(heightsBuffer, sizeof(uint16_t), size, fp);
	for (int i = 0; i<size; i++) {
		mapHeights[i] = heightsBuffer[i] / 65536.0f * 256.0f * 0.05f - 2.445f;
	}
	delete heightsBuffer;
	printf("Building terrain...\n");
	float half_width = 0.5f*256;
	float half_height = 0.5f*256;

	const float inv_height = 1.0f / 256;
	const float inv_width = 1.0f / 256;

	

	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;
	int num_vertices = (256 - 2) * (256 - 2);
	vertices.resize((256 - 2) * (256 - 2));

	//vertices_data
	for (int z = 1; z<256 - 1; z++)
	{
		for (int x = 1; x<256 - 1; x++)
		{
			float fx = (float)x;
			float fz = (float)z;
			//Vertex 1
			//Position
			vertices[((x - 1) + (z - 1)*(256 - 2))].position.x = fx / (256 / terrainDim);
			vertices[((x - 1) + (z - 1)*(256 - 2))].position.y = mapHeights[x + z * 256];
			vertices[((x - 1) + (z - 1)*(256 - 2))].position.z = fz / (256 / terrainDim);
			//TexCoords
			vertices[((x - 1) + (z - 1)*(256 - 2))].textureCoordinate.x = fx * inv_width;
			vertices[((x - 1) + (z - 1)*(256 - 2))].textureCoordinate.y = fz * inv_height;
			//Normal
			vertices[((x - 1) + (z - 1)*(256 - 2))].normal.x = 0;
			vertices[((x - 1) + (z - 1)*(256 - 2))].normal.y = 1;
			vertices[((x - 1) + (z - 1)*(256 - 2))].normal.z = 0;
		}
	}
	//indices data
	for (int z = 1; z<256 - 2; z++)
	{
		for (int x = 1; x<256 - 2; x++)
		{
			indices.push_back((x - 1) + (z - 1) * (256 - 2));
			indices.push_back((x - 1) + z * (256 - 2));
			indices.push_back(x + (z - 1) * (256 - 2));

			indices.push_back((x - 1) + z * (256 - 2));
			indices.push_back(x + z * (256 - 2));
			indices.push_back(x + (z - 1) * (256 - 2));
		}
	}

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
	rmodel->setTexture(device, L"Media/terrainTex.jpg");
	rmodel->setNormalMap(device, L"Media/terrainNormalMap.jpg");
	this->components.push_back(rmodel);
	this->width = 256;
	this->height = 256;
	this->heights = mapHeights;
}

float terrain::GetHeight(float x, float z) const {
	if (x<0 || z<0 || x>terrainDim - 2 || z>terrainDim - 2)
		return 0;
	//float Epsilon = 0.005;
	//float idx = x*height / terrainDim + width*z*width / terrainDim;
	//if ((idx - int(idx))<Epsilon)
	//	return heights[int(idx)];
	//Bilinear Interpolation
	float u = (x * (height / terrainDim) - int(x * (height / terrainDim)));
	float v = (z * (height / terrainDim) - int(z * (height / terrainDim)));

	float y =
		(heights[int(floor(x) * (height / terrainDim) + width * (floor(z) * (height / terrainDim)))] * (1 - u) +
			heights[int(floor(x) * (height / terrainDim) + 1 + width * (floor(z) * (height / terrainDim)))] * u) * (1 - v) +
			(heights[int(floor(x) * (height / terrainDim) + width * (floor(z) * (height / terrainDim) + 1))] * (1 - u) +
				heights[int(floor(x) * (height / terrainDim) + 1 + width * (floor(z) * (height / terrainDim) + 1))] * u) * v;
	return y;
}
