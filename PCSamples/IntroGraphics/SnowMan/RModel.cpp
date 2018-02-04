#include "pch.h"
#include "RModel.h"


RModel::RModel()
{
	model = DirectX::XMMatrixIdentity();
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
}


RModel::~RModel()
{
}
