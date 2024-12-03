Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 fragPos : TEXCOORD2;
};

float4 fs_main(PS_Input i) : SV_TARGET
{
    float2 testUv = i.uv;
    float4 textureColor = srv.Sample(samplerState, testUv);   
    return textureColor;
}