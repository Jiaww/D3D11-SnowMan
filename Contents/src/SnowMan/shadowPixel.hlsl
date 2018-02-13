//------------------------------------------------------------------------------------
// PixelShader.hlsl
//
// Simple shader to render a triangle
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct Interpolants
{
	float4 position     : SV_Position;
	float3 normal       : fs_Nor;
	float2 tex          : TEXCOORD0;
};

float4 main(Interpolants In) : SV_Target
{
	float depthValue;
	float4 color;


	// Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
	depthValue = In.position.z / In.position.w;

	color = float4(depthValue, depthValue, depthValue, 1.0f);

	return color;
}