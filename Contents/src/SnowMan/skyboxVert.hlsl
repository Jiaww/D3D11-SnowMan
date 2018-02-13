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
	float2 tex          : fs_Tex;
};

Interpolants main(Vertex In)
{
	Interpolants Out;
	/* Sky Box Vertex Shader */
	float4 position = float4(In.position + camPos.xyz, 1.0f);
	position = mul(position, viewMatrix);
	position = mul(position, projectionMatrix);
	position.z = position.w*0.99999;
	Out.position = position.xyzw;
	Out.normal = In.normal;
	Out.tex = In.tex;

	return Out;
}