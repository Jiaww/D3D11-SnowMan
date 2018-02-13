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

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct Pixel
{
	float4 color    : SV_Target;
};


Pixel main(Interpolants In)
{
	Pixel Out;
	float4 col = txDiffuse.Sample(samLinear, In.tex);
	Out.color = float4(col.xyz, 1.0);
	return Out;
}