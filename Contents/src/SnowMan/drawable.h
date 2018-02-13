#pragma once
#include "RModel.h"

class drawable
{
public:
	std::vector<RModel*> components;
	virtual void create(ID3D11Device1* device) = 0;
	drawable();
	~drawable();
};

