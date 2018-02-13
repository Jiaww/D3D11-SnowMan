#pragma once
#include "drawable.h"
#include "RModel.h"
#include <GeometricPrimitive.h>
#include "Utilities.h"
#include "FindMedia.h"
#include <comdef.h> 

class terrain : public drawable
{
public:
	terrain();
	~terrain();
	void create(ID3D11Device1* device);
	wchar_t* HP_filename;
	float terrainDim;
	int width, height;
	float *heights;
	float GetHeight(float x, float z) const;
	int GetTerrainDim() const { return terrainDim; }
};

