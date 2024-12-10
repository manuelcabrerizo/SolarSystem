Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input
{
    float4 PosH : SV_Position;
    float4 Color : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};

float4 fs_main(PS_Input i) : SV_TARGET
{
    //return srv.Sample(samplerState, i.Tex) * i.Color;
    return float4(0, 4, 0, 1);

}