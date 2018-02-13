#pragma once
#include "drawable.h"
#include "RModel.h"
#include <GeometricPrimitive.h>
#include "Utilities.h"

class skybox : public drawable
{
public:
	skybox();
	~skybox();
	void create(ID3D11Device1* device);
};

