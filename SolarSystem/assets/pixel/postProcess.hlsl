Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

#define PI 3.14159265359
#define TAU 6.28318530718

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 fragPos : TEXCOORD2;

};

cbuffer Common : register(b3)
{
    float2 resolution;
    float time;
    float pad0;
}

float4 fs_main(PS_Input i) : SV_TARGET
{
    float2 uv = float2(i.uv.x, 1.0f - i.uv.y); 
    float4 textureColor = srv.Sample(samplerState, uv);
    return textureColor;
}