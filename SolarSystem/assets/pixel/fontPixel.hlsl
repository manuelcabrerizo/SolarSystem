Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 fs_main(PS_Input i) : SV_TARGET
{
    float4 textureColor = srv.Sample(samplerState, i.uv); 
    return float4(textureColor.rgb * float3(0, 2, 0), textureColor.a);
}