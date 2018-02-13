#include "pch.h"
#include "RModel.h"
#include "FindMedia.h"
#include "Utilities.h"

RModel::RModel()
{
	model = DirectX::XMMatrixIdentity();
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
}


RModel::~RModel()
{
}

void RModel::setTexture(ID3D11Device1* device, const wchar_t *path) {
	// Create texture.
	D3D11_TEXTURE2D_DESC txtDesc = {};
	txtDesc.MipLevels = txtDesc.ArraySize = 1;
	txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // sunset.jpg is in sRGB colorspace
	txtDesc.SampleDesc.Count = 1;
	txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
	txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	wchar_t buff[MAX_PATH];
	DX::FindMediaFile(buff, MAX_PATH, path);
	auto image = LoadBGRAImage(buff, txtDesc.Width, txtDesc.Height);
	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = image.data();
	initialData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

	ID3D11Texture2D* tex;
	DX::ThrowIfFailed(
		device->CreateTexture2D(&txtDesc, &initialData,
			&tex));

	//tex = CreateDDS
	DX::ThrowIfFailed(
		device->CreateShaderResourceView(tex,
			nullptr, &this->texture));
}

void RModel::setNormalMap(ID3D11Device1* device, const wchar_t *path) {
	// Create texture.
	D3D11_TEXTURE2D_DESC txtDesc = {};
	txtDesc.MipLevels = txtDesc.ArraySize = 1;
	txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // sunset.jpg is in sRGB colorspace
	txtDesc.SampleDesc.Count = 1;
	txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
	txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	wchar_t buff[MAX_PATH];
	DX::FindMediaFile(buff, MAX_PATH, path);
	auto image = LoadBGRAImage(buff, txtDesc.Width, txtDesc.Height);
	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = image.data();
	initialData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

	ID3D11Texture2D* tex;
	DX::ThrowIfFailed(
		device->CreateTexture2D(&txtDesc, &initialData,
			&tex));

	//tex = CreateDDS
	DX::ThrowIfFailed(
		device->CreateShaderResourceView(tex,
			nullptr, &this->normalMap));
}