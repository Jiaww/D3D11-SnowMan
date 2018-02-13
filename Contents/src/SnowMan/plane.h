#pragma once
#include "drawable.h"

class plane : public drawable
{
public:
	plane();
	~plane();
	void create(ID3D11Device1* device);
};

