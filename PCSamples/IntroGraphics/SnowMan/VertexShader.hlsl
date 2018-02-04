//--------------------------------------------------------------------------------------
// VertexShader.hlsl
//
// Simple vertex shader for rendering a triangle
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
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

Interpolants main( Vertex In )
{
	Interpolants Out;
	Out.position = float4(In.position, 1.0f);
	Out.position = mul(Out.position, worldMatrix);
	Out.position = mul(Out.position, viewMatrix);
	Out.position = mul(Out.position, projectionMatrix);
	Out.normal = mul(In.normal.xyz, (float3x3)worldMatrix).xyz;
	Out.normal = normalize(Out.normal);
	/* Sky Box Vertex Shader */
	/*Don't forget to reverse the indices*/
	//float4 position = float4(In.position + camPos.xyz, 1.0f);
	////Out.position = mul(Out.position, worldMatrix);
	//position = mul(position, viewMatrix);
	//position = mul(position, projectionMatrix);
	//position.z = position.w * 0.99;
	//Out.position = position.xyzw;
	//Out.normal = In.normal;

	return Out;
}