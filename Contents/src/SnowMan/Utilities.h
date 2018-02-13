#pragma once
#include <vector>
#include "pch.h"
#include "FindMedia.h"
#include "ReadData.h"

std::vector<uint16_t> reverseIndices(std::vector<uint16_t> indices);
bool IsInBox(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 box[8]);
bool IsIntersectBBox(DirectX::XMFLOAT3 camPos, DirectX::XMFLOAT3 camBBoxHalfWidth, DirectX::XMMATRIX Rotation, DirectX::XMFLOAT3 BBox[8]);
std::vector<uint8_t> LoadBGRAImage(const wchar_t* filename, uint32_t& width, uint32_t& height);