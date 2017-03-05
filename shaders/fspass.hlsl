//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "shared.h"


struct tPixel
{
    float4 mColour;
};
StructuredBuffer<tPixel> ComputeOutputBuffer : register(t0);

Texture2D NoiseTexture : register(t1);
SamplerState NoiseSS : register(s0);


struct PSInput
{
    float4 position : SV_POSITION;
	float2 uv	: TEXCOORD;
};

PSInput VSMain(float4 position : POSITION, float2 uv	: TEXCOORD)
{
	PSInput result;

	result.position = position;
	result.uv = uv;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float2 uv = input.uv;

	uint index = (int)(uv.x * gWidth) + (int)(uv.y * gHeight) * gWidth;

	float4 result;
	result.rgb = ComputeOutputBuffer[index].mColour.rgb;
	result.a = 1.0;

	const float tile_scale = 50;  //50 is a hack to tile many times, seems sufficient
	float dither = NoiseTexture.Sample(NoiseSS, (uv + float2(gNoiseOffsetX, gNoiseOffsetY)) * tile_scale).r;
	dither = dither / 150.0; //150 is a hack, produces a good enough result
	
	result.rgb += dither;

    return result;
}
