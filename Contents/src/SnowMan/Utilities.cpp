#include "Utilities.h"
#include "pch.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;


std::vector<uint16_t> reverseIndices(std::vector<uint16_t> indices) {
	std::vector<uint16_t> reversed;
	for (int i = indices.size() - 1; i >= 0; i--)
		reversed.push_back(indices[i]);
	return reversed;
}

bool IsInBox(XMFLOAT3 pos, XMFLOAT3 box[8]) {
	XMFLOAT3 minP(10000, 10000, 10000);
	XMFLOAT3 maxP(-10000, -10000, -10000);

	for (int i = 0; i < 8; i++) {
		minP.x = XMMin(minP.x, box[i].x);
		minP.y = XMMin(minP.y, box[i].y);
		minP.z = XMMin(minP.z, box[i].z);
		maxP.x = XMMax(maxP.x, box[i].x);
		maxP.y = XMMax(maxP.y, box[i].y);
		maxP.z = XMMax(maxP.z, box[i].z);
	}

	if (pos.x <= maxP.x && pos.y <= maxP.y && pos.z <= maxP.z && pos.x >= minP.x && pos.y >= minP.y && pos.z >= minP.z)
		return true;
	return false;
}

std::vector<uint8_t> LoadBGRAImage(const wchar_t* filename, uint32_t& width, uint32_t& height)
{
	ComPtr<IWICImagingFactory> wicFactory;
	DX::ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory)));

	ComPtr<IWICBitmapDecoder> decoder;
	DX::ThrowIfFailed(wicFactory->CreateDecoderFromFilename(filename, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf()));

	ComPtr<IWICBitmapFrameDecode> frame;
	DX::ThrowIfFailed(decoder->GetFrame(0, frame.GetAddressOf()));

	DX::ThrowIfFailed(frame->GetSize(&width, &height));

	WICPixelFormatGUID pixelFormat;
	DX::ThrowIfFailed(frame->GetPixelFormat(&pixelFormat));

	uint32_t rowPitch = width * sizeof(uint32_t);
	uint32_t imageSize = rowPitch * height;

	std::vector<uint8_t> image;
	image.resize(size_t(imageSize));

	if (memcmp(&pixelFormat, &GUID_WICPixelFormat32bppBGRA, sizeof(GUID)) == 0)
	{
		DX::ThrowIfFailed(frame->CopyPixels(0, rowPitch, imageSize, reinterpret_cast<BYTE*>(image.data())));
	}
	else
	{
		ComPtr<IWICFormatConverter> formatConverter;
		DX::ThrowIfFailed(wicFactory->CreateFormatConverter(formatConverter.GetAddressOf()));

		BOOL canConvert = FALSE;
		DX::ThrowIfFailed(formatConverter->CanConvert(pixelFormat, GUID_WICPixelFormat32bppBGRA, &canConvert));
		if (!canConvert)
		{
			throw std::exception("CanConvert");
		}

		DX::ThrowIfFailed(formatConverter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGRA,
			WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut));

		DX::ThrowIfFailed(formatConverter->CopyPixels(0, rowPitch, imageSize, reinterpret_cast<BYTE*>(image.data())));
	}

	return image;
}

bool IsIntersectBBox(DirectX::XMFLOAT3 camPos, DirectX::XMFLOAT3 camBBoxHalfWidth, DirectX::XMMATRIX Rotation, DirectX::XMFLOAT3 BBox[8]) {
	//Check if in Box
	XMFLOAT3 camBox[8];
	camBox[0] = XMFLOAT3(camPos.x - camBBoxHalfWidth.x, camPos.y - camBBoxHalfWidth.y, camPos.z - camBBoxHalfWidth.z);
	camBox[1] = XMFLOAT3(camPos.x + camBBoxHalfWidth.x, camPos.y - camBBoxHalfWidth.y, camPos.z - camBBoxHalfWidth.z);
	camBox[2] = XMFLOAT3(camPos.x - camBBoxHalfWidth.x, camPos.y + camBBoxHalfWidth.y, camPos.z - camBBoxHalfWidth.z);
	camBox[3] = XMFLOAT3(camPos.x - camBBoxHalfWidth.x, camPos.y - camBBoxHalfWidth.y, camPos.z + camBBoxHalfWidth.z);
	camBox[4] = XMFLOAT3(camPos.x + camBBoxHalfWidth.x, camPos.y + camBBoxHalfWidth.y, camPos.z - camBBoxHalfWidth.z);
	camBox[5] = XMFLOAT3(camPos.x + camBBoxHalfWidth.x, camPos.y - camBBoxHalfWidth.y, camPos.z + camBBoxHalfWidth.z);
	camBox[6] = XMFLOAT3(camPos.x - camBBoxHalfWidth.x, camPos.y + camBBoxHalfWidth.y, camPos.z + camBBoxHalfWidth.z);
	camBox[7] = XMFLOAT3(camPos.x + camBBoxHalfWidth.x, camPos.y + camBBoxHalfWidth.y, camPos.z + camBBoxHalfWidth.z);

	for (int i = 0; i < 8; i++) {
		XMFLOAT3 InvCamPos;
		XMStoreFloat3(&InvCamPos, XMVector3Transform(XMLoadFloat3(&camBox[i]), XMMatrixInverse(nullptr, Rotation)));
		if (IsInBox(InvCamPos, BBox))
			return true;
	}
	return false;
}