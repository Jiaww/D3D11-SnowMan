#pragma once
#include "drawable.h"
#include "RModel.h"
#include <GeometricPrimitive.h>
#include "Utilities.h"

class snowMan : public drawable 
{
public:
	snowMan();
	~snowMan();
	void create(ID3D11Device1* device);
};

