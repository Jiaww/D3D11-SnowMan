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
	float4 lightPosition: TEXCOORD1;
};

Interpolants main( Vertex In )
{
	Interpolants Out;
	Out.position = float4(In.position, 1.0f);
	Out.position = mul(Out.position, worldMatrix);
	Out.position = mul(Out.position, viewMatrix);
	Out.position = mul(Out.position, projectionMatrix);
	Out.normal = mul(In.normal.xyz, (float3x3)invTransWorldMatrix).xyz;
	Out.normal = normalize(Out.normal);
	Out.tex = In.tex;
	Out.lightPosition = float4(In.position, 1.0f);
	Out.lightPosition = mul(Out.lightPosition, worldMatrix);
	Out.lightPosition = mul(Out.lightPosition, lightViewMatrix);
	Out.lightPosition = mul(Out.lightPosition, lightProjectionMatrix);

	return Out;
}