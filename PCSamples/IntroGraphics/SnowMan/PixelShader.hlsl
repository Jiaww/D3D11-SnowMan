//------------------------------------------------------------------------------------
// PixelShader.hlsl
//
// Simple shader to render a triangle
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
cbuffer CameraBuffer
{
	float4 c_color;
};

struct Interpolants
{
	float4 position     : SV_Position;
	float3 normal       : fs_Nor;
	float2 tex          : fs_Tex;
};


struct Pixel
{
    float4 color    : SV_Target;
};


Pixel main( Interpolants In )
{
    Pixel Out;
	float3 lightDir = float3(1.0, 1.0, -1.0);

	float diffuseTerm = saturate(dot(In.normal, normalize(lightDir.xyz)));
	float ambientTerm = 0.05;
    Out.color = float4((ambientTerm + diffuseTerm) * c_color.xyz, 1.0);
    return Out;
}