#pragma once
#include "drawable.h"
#include "RModel.h"
#include <GeometricPrimitive.h>
#include "Utilities.h"

class cube : public drawable
{
public:
	cube();
	~cube();
	void create(ID3D11Device1* device);
};

