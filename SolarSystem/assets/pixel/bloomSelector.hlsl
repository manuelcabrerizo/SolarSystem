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
    float3 hdrColor = srv.Sample(samplerState, float2(i.uv.x, 1.0f - i.uv.y)).rgb;

    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    float brightness = dot(hdrColor.rgb, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        result = float4(hdrColor.rgb, 1.0);
    else
        result = float4(0.0, 0.0, 0.0, 1.0);

    return result;
}


