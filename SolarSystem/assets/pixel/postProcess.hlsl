Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

#define PI 3.14159265359
#define TAU 6.28318530718

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewDir : TEXTCOORD1;
};

float4 fs_main(PS_Input i) : SV_TARGET
{
    float4 textureColor = srv.Sample(samplerState, i.uv);
    return textureColor;
}