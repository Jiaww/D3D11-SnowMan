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
	float2 tex          : TEXCOORD0;
	float4 lightPosition: TEXCOORD1;
};


struct Pixel
{
	float4 color    : SV_Target0;
};

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

Texture2D txNormal : register(t1);
SamplerState samLinear2 : register(s1);

Texture2D txShadowMap : register(t2);
SamplerState samLinear3: register(s2);

float2 texOffset(int u, int v)
{
	return float2(u * 1.0f / 1024.0, v * 1.0f / 1024.0);
}


float shadow(float3 lpos) {
	// Outside light
	if (lpos.x < -1.0f || lpos.x > 1.0f ||
		lpos.y < -1.0f || lpos.y > 1.0f ||
		lpos.z < 0.0f || lpos.z > 1.0f)
		return 1.0;
	//transform clip space coords to texture space coords (-1:1 to 0:1)
	lpos.x = lpos.x / 2.0 + 0.5;
	lpos.y = lpos.y / -2.0 + 0.5;
	float shadowMapBias = 0.0001;
	lpos.z -= shadowMapBias;
	//sample shadow map - point sampler
	//basic hardware PCF - single texel
	float shadowFactor = 1.0;
	//PCF sampling for shadow map
	float sum = 0;
	float x, y;

	//perform PCF filtering on a 4 x 4 texel neighborhood
	for (y = -1.5; y <= 1.5; y += 1.0)
	{
		for (x = -1.5; x <= 1.5; x += 1.0)
		{
			float shadowMapDepth = txShadowMap.Sample(samLinear3, lpos.xy + texOffset(x, y)).r;

			//if clip space z value greater than shadow map value then pixel is in shadow
			if (shadowMapDepth < lpos.z)
				sum += 0.2;
			else
				sum += 1.0;
		}
	}

	shadowFactor = sum / 16.0;

	return shadowFactor;
}

Pixel main(Interpolants In)
{
	In.lightPosition.xyz /= In.lightPosition.w;
	float sd = shadow(In.lightPosition.xyz);

	// Local normal, in tangent space
	float3 TextureNormal_tangentspace;
	TextureNormal_tangentspace = normalize(txNormal.Sample(samLinear2, In.tex).xyz*2.0f - 1.0f);
	float3 TextureNormal_worldspace;
	TextureNormal_worldspace.y = TextureNormal_tangentspace.z;
	TextureNormal_worldspace.z = -TextureNormal_tangentspace.y;
	TextureNormal_worldspace.x = TextureNormal_tangentspace.x;

	Pixel Out;
	float3 lightDir = float3(-1.0, 1.0, -1.0);
	float diffuseTerm = saturate(dot(TextureNormal_worldspace, normalize(lightDir.xyz)));
	float ambientTerm = 0.1;
	float4 col = txDiffuse.Sample(samLinear, In.tex);
	Out.color = float4(sd*(ambientTerm + diffuseTerm)*col.xyz, 1.0);
	return Out;
}