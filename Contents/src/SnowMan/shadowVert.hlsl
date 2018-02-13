//--------------------------------------------------------------------------------------
// VertexShader.hlsl
//
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix invTransWorldMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

cbuffer CameraBuffer
{
	float4 camPos;
};

struct Vertex
{
	float3 position      : vs_Pos;
	float3 normal        : vs_Nor;
	float2 tex           : vs_Tex;
};

struct Interpolants
{
	float4 position     : SV_Position;
	float3 normal       : fs_Nor;
	float2 tex          : TEXCOORD0;
};

Interpolants main(Vertex In)
{
	Interpolants Out;
	Out.position = float4(In.position, 1.0f);
	Out.position = mul(Out.position, worldMatrix);
	Out.position = mul(Out.position, lightViewMatrix);
	Out.position = mul(Out.position, lightProjectionMatrix);
	return Out;
}